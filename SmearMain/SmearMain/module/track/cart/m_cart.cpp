#include "m_cart.h"
#include "rack/rt_rack_manager.h"
#include "carts_manager.h"
#include "track/m_export.h"
#include "smear/m_sampling.h"
#include "f_common.h"

#ifdef Q_OS_WIN
#define Debug_Main
#endif

QMap<QString, int> CartCoord::CoordBaseMap = {
    {"Reset",      0},
    {"PosImport",  10},
    {"PosScan",    12},
    {"PosTest",    14},
    {"PosExport",  27}
};

CartCoord::CartCoord(): pos(Pos_Origin) {}

CartCoord::CartCoord(const QString &name) : pos(Pos_Origin), m_name(name) {}

void CartCoord::setCoord(int x)
{
    coord = x;
    left = x;
    right = x - Width;
    qDebug().noquote() << QString("[%1] Coord:[%2,%3]").arg(m_name).arg(coord).arg(right);
}

void CartCoord::setRange(int to)
{
    left = qMax(coord, to);
    right = qMin(coord, to) - Width;
    qDebug().noquote() << QString("[%1] Move: %2 -> %3, coord range:(%4,%5)")
                          .arg(m_name).arg(coord).arg(to).arg(left).arg(right);
}

QString CartCoord::getCoorStr()
{
    return QString("(%1, %2)").arg(left).arg(right);
}

/******************************** MCart ********************************/

MCart::MCart(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    m_isEnable = true;
    m_suspend = false;
    mRelatePos = RePos_None;
    mCoord.setName(userid);
    m_scanDuration = FCommon::GetInstance()->getConfigValue("track", "scan_duration").toInt(2200);

    mMainTimer = new QTimer(this);
    mMainTimer->setInterval(50);
    connect(mMainTimer, &QTimer::timeout, this, &MCart::processTimer_slot);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(1000);
    connect(m_timeoutTimer, &QTimer::timeout, this, &MCart::cancelTimeoutTimer_slot);

    mRack = nullptr;
    state_init();

    auto track = RtDeviceManager::GetInstance()->track();
    connect(track, &DTrack::onFunctionFinished_signal,
            this, &MCart::onFunctionFinished_slot);
}

void MCart::state_init()
{
    mLock = false;
    m_isLocked = false;
    m_isResetting = false;
    m_isAborted = false;
    m_hasAvoidRequest = false;
    m_isHighPriority = false;
    m_timeoutCount = 0;

    s_MainProcess = MainProcess::Idle;

    mScanSample = nullptr;
    s_Scan = ScanState::Idle;

    mSendSample = nullptr;
    s_Send = SendState::Idle;

    mRecycleSample = nullptr;
    s_RecycleSample = RecycleSampleState::Idle;

    s_RecycleRack = RecycleRackState::Idle;

    qDebug() << "Cart" << userid() << __FUNCTION__;
}

void MCart::start()
{
    m_suspend = false;

    if (mMainTimer->isActive() == false) {
        mMainTimer->start();
        DModuleBase::start();
    }
}

void MCart::reset()
{
	mRack = nullptr;
    mRelatePos = RePos_None;

    state_init();
    DModuleBase::reset();
}

void MCart::stop()
{
    mMainTimer->stop();
    DModuleBase::stop();
}

void MCart::abort()
{
    m_isAborted = true;

    if (mRack) {
        mRack->abort();
    }
}

void MCart::setRackCoord(int coord)
{
    if (mRack) {
        mRack->setCoord(coord);
    }
}

bool MCart::permitAlliedBack()
{
    if (mRack && mRack->getUnFinishedPos() > 8 && mCoord.coord > 21) {
        return true;
    }
    return false;
}

void MCart::cmd_Reset()
{
    if (CartCoord::CoordBaseMap.contains("Reset") == false) {
        qFatal("Can NOT find Reset coord.");
    }
    bool ok = cmd_CReset();
    if (ok) {
        m_isResetting = true;
        int to = CartCoord::CoordBaseMap.value("Reset");
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosImport()
{
    if (CartCoord::CoordBaseMap.contains("PosImport") == false) {
        qFatal("Can NOT find PosImport coord.");
    }
    bool ok = cmd_CPosImport();
    if (ok) {
        int to = CartCoord::CoordBaseMap.value("PosImport");
        Q_ASSERT(to > 0);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosScan(int pos)
{
	if (CartCoord::CoordBaseMap.contains("PosScan") == false) {
        qFatal("Can NOT find PosScan coord.");
    }
    bool ok = cmd_CPosScan(pos);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value("PosScan") + pos;
        Q_ASSERT(to > pos);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosTest(int pos)
{
	if (CartCoord::CoordBaseMap.contains("PosTest") == false) {
        qFatal("Can NOT find PosTest coord.");
    }
    bool ok = cmd_CPosTest(pos);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value("PosTest") + pos;
        Q_ASSERT(to > pos);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosExport()
{
    if (CartCoord::CoordBaseMap.contains("PosExport") == false) {
        qFatal("Can NOT find PosExport coord.");
    }
    bool ok = cmd_CPosExport();
    if (ok) {
        int to = CartCoord::CoordBaseMap.value("PosExport");
        Q_ASSERT(to > 0);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosLeft(int offset)
{
    bool ok = cmd_CPosLeft(offset);
    if (ok) {
        mCoord.toCoord = mCoord.coord + offset;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosRight(int offset)
{
    bool ok = cmd_CPosRight(offset);
    if (ok) {
        mCoord.toCoord = mCoord.coord - offset;
        m_timeoutCount = 0;
    }
}

/*****************************************************************************************/

QSharedPointer<RtRack> MCart::newRack()
{
    if (mRack == nullptr && mCoord.pos == Pos_Import) {
        mRack = RtRackManager::GetInstance()->NewRackObj();
        mRack->setCartId(mid());

        state_init();
        mMainTimer->start();
//        m_timeoutTimer->start();
        return mRack;
    }
    return nullptr;
}

//! [Main]
void MCart::processTimer_slot()
{
    if (dev->track()->isResetOk() == false) {
        return;
    }

    switch (s_MainProcess) {
    case MainProcess::Idle:
        if (mRack != nullptr) {
#ifdef Debug_Main
            qDebug() << mUserId << "Main 0 Idle";
#endif
            if (mLock == true) {
                m_isLocked = true;
                logProcess("Main", -1, "Lock");
                s_MainProcess = MainProcess::Wait_Another_Cart;
            } else if (mRack->isCanceled()) {
                s_MainProcess = MainProcess::Recycle_Rack;
            } else {
                s_MainProcess = MainProcess::Recycle_Sample;
            }
        }
        break;

    case MainProcess::Wait_Another_Cart:
        if (mLock == false) {
            logProcess("Main", -1, "Unlock");
            m_isLocked = false;
            s_MainProcess = MainProcess::Idle;
        }
        break;

        /* 1.Recieve Sample From Station */
    case MainProcess::Recycle_Sample:
        if (s_RecycleSample == RecycleSampleState::Finish) {
            s_MainProcess = MainProcess::Send_Sample;
        } else {
#ifdef Debug_Main
            qDebug() << mUserId << "Main 1 Recycle Sample";
#endif
            RecycleSampleFrStat();
            if (s_RecycleSample > RecycleSampleState::Idle) {
                s_MainProcess = MainProcess::WaitF_1;
            } else {
                s_MainProcess = MainProcess::Send_Sample;
            }
        }
        break;
    case MainProcess::WaitF_1:
        if (mLock == true) {
            if (s_RecycleSample > RecycleSampleState::Move_To_Station_Pos) {
                RecycleSampleFrStat();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        } else {
            if (s_RecycleSample > RecycleSampleState::Idle) {
                RecycleSampleFrStat();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        }
        break;

        /* 2.Send Sample To Station */
    case MainProcess::Send_Sample:
        if (s_Send == SendState::Finish) {
            s_MainProcess = MainProcess::Scan_Sample;
        } else {
#ifdef Debug_Main
            qDebug() << mUserId << "Main 2 Send Sample";
#endif
            SendToStatProcess();
            if (s_Send > SendState::Idle) {
                s_MainProcess = MainProcess::WaitF_2;
            } else {
                s_MainProcess = MainProcess::Scan_Sample;
            }
        }
        break;
    case MainProcess::WaitF_2:
        if (mLock == true) {
            if (s_Send > SendState::Move_To_Test_Pos) {
                SendToStatProcess();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        } else {
            if (s_Send > SendState::Idle) {
                SendToStatProcess();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        }
        break;

        /* 3.Scan Sample */
    case MainProcess::Scan_Sample:
        if (s_Scan == ScanState::Finish) {
            s_MainProcess = MainProcess::Recycle_Rack;
        } else {
#ifdef Debug_Main
            qDebug() << mUserId << "Main 3 Scan Sample";
#endif
            ScanBarcodeProcess();
            if (s_Scan > ScanState::Idle) {
                s_MainProcess = MainProcess::WaitF_3;
            } else {
                s_MainProcess = MainProcess::Recycle_Rack;
            }
        }
        break;
    case MainProcess::WaitF_3:
        if (mLock == true) {
            if (s_Scan > ScanState::Move_To_Scan_Pos) {
                ScanBarcodeProcess();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        } else {
            if (s_Scan > ScanState::Idle) {
                ScanBarcodeProcess();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        }
        break;

        /* 4.Recycle Rack To Export */
    case MainProcess::Recycle_Rack:
#ifdef Debug_Main
        qDebug() << mUserId << "Main 5 Recycle Rack";
#endif
        RecycleRack();
        if (s_RecycleRack > RecycleRackState::Idle) {
            s_MainProcess = MainProcess::WaitF_4;
        } else {
            s_MainProcess = MainProcess::Avoid;
        }
        break;
    case MainProcess::WaitF_4:
        if (s_RecycleRack > RecycleRackState::Idle) {
            RecycleRack();
            if (s_RecycleRack == RecycleRackState::Finish) {
                logProcess("Main", 5, "Reset");
                s_MainProcess = MainProcess::Reset;
            }
        }
        break;

    case MainProcess::Avoid:
        if (mRelatePos == RePos_Front
                && m_hasAvoidRequest
                && mCoord.pos < Pos_Export) {
            bool ok = req_PosExport();
            if (ok) {
                s_MainProcess = MainProcess::WaitF_Avoid_Done;
            }
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;
    case MainProcess::WaitF_Avoid_Done:
        if (isCmd_PosExport_done()) {
            s_MainProcess = MainProcess::Idle;
        }
        break;

    case MainProcess::Reset:
        if (mCoord.pos == Pos_Export) {
            bool ok = req_Reset();
            if (ok) {
                logProcess("Main", 99, "All_Finish");
#ifdef Debug_Main
            qDebug() << mUserId << "Main 99 All_Finish";
#endif
                mMainTimer->stop();
            	m_timeoutTimer->stop();

                mRack = nullptr;
                s_MainProcess = MainProcess::All_Finish;
            }
        }
        break;

    case MainProcess::All_Finish:
        break;
    }
}

//! [1.Scan Code]
void MCart::ScanBarcodeProcess()
{
    switch (s_Scan) {
    case ScanState::Finish:
        break;

    case ScanState::Idle:
        if (m_isAborted == false) {
            int pos = mRack->getNextScanPos();
            if (pos >= 0) {
                mScan_pos = pos;
                logProcess("Scan", 1, "Idle", QString("pos:%1").arg(mScan_pos));
                s_Scan = ScanState::Move_To_Scan_Pos;
            } else {
                dev->track()->setCheckSensorEnable(mid(), false);
                s_Scan = ScanState::Finish;
            }
        } else {
            s_Scan = ScanState::Finish;
        }
        break;

    case ScanState::Move_To_Scan_Pos:
        if (m_isAborted == false) {
            bool ok = req_PosScan(mScan_pos);
            if (ok) {
                logProcess("Scan", 2, "Move_To_Scan_Pos", QString("rack pos:%1").arg(mScan_pos));
                s_Scan = ScanState::WaitF_Pos_Done;
            }
        } else {
            s_Scan = ScanState::Done;
        }
        break;

    case ScanState::WaitF_Pos_Done:
        if (isCmd_PosScan_done()) {
            logProcess("Scan", 3, "Sample_Exist_Check");
            s_Scan = ScanState::Sample_Exist_Check;
        }
        break;

    case ScanState::Sample_Exist_Check:
        dev->track()->cmd_CheckSensorValue();
        s_Scan = ScanState::WaitF_Sample_Exist_Check;
        break;
    case ScanState::WaitF_Sample_Exist_Check:
        if (m_isAborted == true) {
            s_Scan = ScanState::Done;
        } else if (dev->track()->isFuncDone("CheckSensorValue")) {
            bool exist = dev->track()->checkSensorValue("scan_has_sample");

            mRack->setSampleExist(mScan_pos, exist);
            if (exist == true) {
                mScanSample = mRack->addNewSample(mScan_pos);
                logProcess("Scan", 4, "New Sample", QString("pos:%1 sample:%2").arg(mScan_pos).arg(mScanSample->sid()));
                s_Scan = ScanState::Send_Scan_Cmd;
            } else {
                logProcess("Scan", 4, "None", QString("pos:%1").arg(mScan_pos));
                s_Scan = ScanState::Done;
            }

            JPacket p(PacketType::Notification);
            p.module = "Rack";
            p.api = "Sample";

            QJsonObject obj;
            obj.insert("rack_id", mRack->rackid());
            obj.insert("rack_pos", mScan_pos);
            obj.insert("exist", exist);
            p.paramsValue = obj;
            sendUIMessage(p);
        } else if (dev->track()->getFuncResult("CheckSensorValue") > Func_Done) {
            s_Scan = ScanState::Sample_Exist_Check;
        }
        break;

    case ScanState::Send_Scan_Cmd:
        m_scanBarCode.clear();
        dev->sample()->cmd_RotateTube();
        dev->track()->cmd_Scan_Open(m_scanDuration);

        /* record */
        recordModuleSampleState("scan", "doing_sample", mScanSample);

        s_Scan = ScanState::WaitF_Scan_Cmd_Done;
        break;
    case ScanState::WaitF_Scan_Cmd_Done:
        if (dev->sample()->isFuncDone("RotateTube") && dev->track()->isFuncDone("Scan_Open")) {
            mScanSample->setStatus(SampleStatus::Scaned, 1);
            mScanSample->setBarcode(m_scanBarCode);

            /* record */
            recordModuleSampleState("scan", "doing_sample", nullptr);

            RtSampleManager::GetInstance()->requestSampleOrder(mScanSample);
            s_Scan = ScanState::Done;
        }
        break;

    case ScanState::Done:
        mScanSample = nullptr;
        logProcess("Scan", 99, "Done", QString("pos:%1").arg(mScan_pos));
        s_Scan = ScanState::Idle;
        break;
    }
}

//! [2.Send Sample to Test]
void MCart::SendToStatProcess()
{
    switch (s_Send) {
    case SendState::Finish:
        break;

    case SendState::Idle:
        if (m_suspend == false && m_isAborted == false)
        {
            auto list = mRack->getUnSendToTestList();
            if (list.isEmpty() == false) {
                auto sample = list.first();
                if (mSampling->isReceivable(sample->isRet())) {
                    mSendSample = sample;
                    mSendSample->setStatus(SampleStatus::Sending_To_Test, 1);
                    logProcess("Send", 1, "Sending", QString("Sample:%1").arg(mSendSample->sid()));
                    s_Send = SendState::Move_To_Test_Pos;
                }
                break;
            }
        }
        break;

    case SendState::Move_To_Test_Pos:
    {
        bool ok = req_PosTest(mSendSample->rack_pos());
        if (ok) {
            s_Send = SendState::WaitF_Test_Pos_Done;
        }
        break;
    }

    case SendState::WaitF_Test_Pos_Done:
        if (isCmd_PosTest_done()) {
            logProcess("Send", 2, "Arrive_Station", QString("[Rack:%1-%2]")
                       .arg(mRack->rackid()).arg(mSendSample->rack_pos()));
            s_Send = SendState::Send_New_Sample_To_Station;
        }
        break;

    case SendState::Send_New_Sample_To_Station:
    {
        bool ok = mSampling->receiveNewSample(mSendSample);
        if (ok) {
            logProcess("Send", 3, "Send_New_Sample_To_Station", QString("sample:%1").arg(mSendSample->sid()));
            s_Send = SendState::WaitF_Send_Sample_To_Station_Done;
        }
        break;
    }

    case SendState::WaitF_Send_Sample_To_Station_Done:
        if (mSampling->isReceiveFinished()) {
            logProcess("Send", 99, "Finish");
            s_Send = SendState::Done;
        }
        break;
    case SendState::Done:
        mSendSample = nullptr;
        s_Send = SendState::Idle;
        break;
    }
}

//! [3.Recycle Sample From Station]
void MCart::RecycleSampleFrStat()
{
    switch (s_RecycleSample) {
    case RecycleSampleState::Finish:
        break;

    case RecycleSampleState::Idle:
    {
        auto sample = mRack->getUnRecycleSample();
        if (sample) {
            mRecycleSample = sample;
            logProcess("Recycle Sample", 1, "Move_To_Station_Pos",
                       QString("recycle sample:%1").arg(mRecycleSample->sid()));
            s_RecycleSample = RecycleSampleState::Move_To_Station_Pos;
        }
        break;
    }

    case RecycleSampleState::Move_To_Station_Pos:
    {
        bool ok = req_PosTest(mRecycleSample->rack_pos());
        if (ok) {
            s_RecycleSample = RecycleSampleState::WaitF_Move_To_Station_Pos;
        }
        break;
    }
    case RecycleSampleState::WaitF_Move_To_Station_Pos:
        if (isCmd_PosTest_done()) {
            logProcess("Recycle Sample", 2, "Send_Recycle_Sample");
            s_RecycleSample = RecycleSampleState::Send_Recycle_Sample;
        }
        break;

    case RecycleSampleState::Send_Recycle_Sample:
    {
        bool ok = mSampling->sendRecycleSample(mRecycleSample);
        if (ok) {
            s_RecycleSample = RecycleSampleState::WaitF_Recyle_Sample_Done;
        }
        break;
    }
    case RecycleSampleState::WaitF_Recyle_Sample_Done:
        if (mSampling->isRecycleFinished()) {
            mRecycleSample->setStatus(SampleStatus::Recycle_To_Rack, 1);
            logProcess("Recycle Sample", 99, "Finish");
            s_RecycleSample = RecycleSampleState::Done;
        }
        break;

    case RecycleSampleState::Done:
        mRecycleSample = nullptr;

        if (mRack->isAllRecycleFinished()) {
            s_RecycleSample = RecycleSampleState::Finish;
        } else {
            s_RecycleSample = RecycleSampleState::Idle;
        }
        break;
    }
}

//! [4.Recycle Rack]
void MCart::RecycleRack()
{
    switch (s_RecycleRack) {
    case RecycleRackState::Idle:
        if (mRack->isAllTestFinished() || mRack->isCanceled()) {
            logProcess("Recycle Rack", 0, "Start");
            s_RecycleRack = RecycleRackState::Get_Export;
        }
        break;

    case RecycleRackState::Get_Export:
        logProcess("Recycle Rack", 1, "Get_Export", QString("recycle rack:%1").arg(mRack->rackid()));
        s_RecycleRack = RecycleRackState::Move_To_Export_Pos;
        break;

    case RecycleRackState::Move_To_Export_Pos:
    {
        bool ok = req_PosExport();
        if (ok) {
            logProcess("Recycle Rack", 2, "Move_To_Export_Pos");
            s_RecycleRack = RecycleRackState::WaitF_Export_Pos_Done;
        }
        break;
    }
    case RecycleRackState::WaitF_Export_Pos_Done:
        if (isCmd_PosExport_done()) {
            s_RecycleRack = RecycleRackState::Req_Recycle_Rack;
        };
        break;

    case RecycleRackState::Req_Recycle_Rack:
    {
        bool ok = mExport->recycleRack(mRack);
        if (ok) {
            logProcess("Recycle Rack", 3, "Req_Recycle_Rack");
            s_RecycleRack = RecycleRackState::WaitF_Recycle_Rack_Done;
        }
        break;
    }
    case RecycleRackState::WaitF_Recycle_Rack_Done:
        if (mExport->isRecycleRackFinished()) {
            logProcess("Recycle Rack", 99, "Finish");
            s_RecycleRack = RecycleRackState::Finish;
        }
        break;

    case RecycleRackState::Finish:
        break;
    }
}

void MCart::cancelTimeoutTimer_slot()
{
    if (mRelatePos == RePos_Front || mRelatePos == RePos_Single) {
        if (mCoord.pos >= Pos_Origin && m_isLocked == false) {
            m_timeoutCount++;
            if (m_timeoutCount > 120) {
                mRack->setCanceled();
                s_MainProcess = MainProcess::Recycle_Rack;
                m_timeoutTimer->stop();
            }
        }
    }
}

void MCart::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api.startsWith(mUserId)) {
        updatePosition(api);
        onFunctionFinished(api);
    } else if (api == "Reset") {
        m_isResetting = false;
        mCoord.pos = Pos_Origin;
        mCoord.setCoord(0);
        mRelatePos = MCart::RePos_None;
    } else if (api == "Scan_Open") {
        m_scanBarCode.clear();
        if (resValue.isObject()) {
            QJsonObject obj = resValue.toObject();
            m_scanBarCode = obj.value("string").toString().simplified();
        }
    }
}
