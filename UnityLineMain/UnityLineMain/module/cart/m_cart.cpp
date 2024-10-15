#include "m_cart.h"
#include "rack/rt_rack_manager.h"
#include "carts_manager.h"
#include "station/station_manager.h"
#include "module_manager.h"
#include "exit/m_export1.h"
#include "exit/m_export2.h"
#include "f_common.h"

#ifdef Q_OS_WIN
#define Debug_Main
#endif

QMap<QString, int> CartCoord::CoordBaseMap = {
    {"Reset", 0},

    {"PosImport", 10},

    {"PosS1Scan", 12},
    {"PosS1ScanRack", 12 + 1},
    {"PosS1Test", 12 + 2},

    {"PosExit_E1", 27},

    {"PosS2Scan", 34},
    {"PosS2ScanRack", 34 + 1},
    {"PosS2Test", 34 + 2},

    {"PosExit_E2", 49}
};

CartCoord::CartCoord()
    : pos(Pos_Origin)
{}

CartCoord::CartCoord(const QString &name)
    : pos(Pos_Origin)
    , m_name(name)
{}

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
    QString str = QString("(%1, %2)").arg(left).arg(right);
    return str;
}

/******************************** MCart ********************************/

MCart::MCart(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    mStatMgr = StationManager::GetInstance();

    m_isEnable = true;
    m_suspend = false;
    mRelatePos = RePos_None;
    mCoord.setName(userid);
    m_MaxWaiting = FCommon::GetInstance()->getConfigValue("general", "max_waiting_time_sec").toInt();
    qDebug() << mUserId << "Max waiting Time:" << m_MaxWaiting << "sec";

    mMainTimer = new QTimer(this);
    mMainTimer->setInterval(50);
    connect(mMainTimer, &QTimer::timeout, this, &MCart::mainProcess_slot);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(1000);
    connect(m_timeoutTimer, &QTimer::timeout, this, &MCart::cancelTimeoutTimer_slot);

    m_reviewTimeoutTimer = new QTimer(this);
    m_reviewTimeoutTimer->setInterval(2 * 60 * 1000);
    connect(m_reviewTimeoutTimer, &QTimer::timeout, this, &MCart::onReviewTimeout_slot);

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
    s_ReviewRack = ReviewRackState::Idle;

    mScanStation = nullptr;
    m_isScanRackCodeFinished = false;
    s_Scan = ScanState::Scan_RackCode_Idle;

    mUnsendSampleList.clear();
    mTodoSample = nullptr;
    mSendSample = nullptr;
    mSendStation = nullptr;
    s_Send = SendState::Idle;

    mRecycleSample = nullptr;
    mRecycleStation = nullptr;
    s_RecycleSample = RecycleSampleState::Idle;

    mExport = nullptr;
    s_RecycleRack = RecycleRackState::Idle;

    logProcess("Reset", 0, "state_init");
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
    if (mCoord.coord <= 16) {
        return false;
    }

    if (mRack) {
        if (mRack->isCanceled()) {
            return true;
        } else if (mRack->minReviewDonePos() > 8) {
            return true;
        }
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

void MCart::cmd_PosSxScanRackCode(const QString &sid)
{
    QString api = QString("Pos%1ScanRack").arg(sid);
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qFatal("Can NOT find PosSxScanRack coord.");
    }
    bool ok = cmd_CPosScanRackCode(sid);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value(api);
        Q_ASSERT(to > 0);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosSxScan(const QString &sid, int pos)
{
    QString api = QString("Pos%1Scan").arg(sid);
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosSxScan coord.");
    }
    bool ok = cmd_CPosSxScan(sid, pos);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value(api) + pos;
        Q_ASSERT(to > pos);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosSxTest(const QString &sid, int pos)
{
    QString api = QString("Pos%1Test").arg(sid);
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosSxTest coord.");
    }
    bool ok = cmd_CPosSxTest(sid, pos);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value(api) + pos;
        Q_ASSERT(to > pos);
        mCoord.toCoord = to;
        m_timeoutCount = 0;
    }
}

void MCart::cmd_PosExit_Ex(const QString &eid)
{
    QString api = QString("PosExit_%1").arg(eid);
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosExit_Ex coord.");
    }
    bool ok = cmd_CPosExit_Ex(api);
    if (ok) {
        int to = CartCoord::CoordBaseMap.value(api);
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

void MCart::cmd_Scan_Open(int index, int timeout)
{
    if (index == 1) {
        dev->track()->cmd_S1_Scan_Open(timeout);
    } else if (index == 2) {
        dev->track()->cmd_S2_Scan_Open(timeout);
    } else if (index == 3) {
        dev->track()->cmd_S3_Scan_Open(timeout);
    }
}

bool MCart::isScanOpen_Done(int index)
{
    if (index == 1) {
        return dev->track()->isFuncDone("S1_Scan_Open");
    } else if (index == 2) {
        return dev->track()->isFuncDone("S2_Scan_Open");
    } else if (index == 3) {
        return dev->track()->isFuncDone("S3_Scan_Open");
    }
    return false;
}

/*****************************************************************************************/

void MCart::newRack()
{
    if (mRack == nullptr && mCoord.pos == Pos_Import) {
        mRack = RtRackManager::GetInstance()->NewRackObj();
        mRack->setCartId(mid());

        state_init();
        mMainTimer->start();
        m_timeoutTimer->start();
    }
}

//! [Main]
void MCart::mainProcess_slot()
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
                s_MainProcess = MainProcess::Scan_Sample;
            }
        }
        break;

        /* 3.Scan Sample */
    case MainProcess::Scan_Sample:
        if (s_Scan == ScanState::Finish) {
            s_MainProcess = MainProcess::Review_Rack;
        } else {
#ifdef Debug_Main
            qDebug() << mUserId << "Main 3 Scan Sample";
#endif
            ScanBarcodeProcess();
            if (s_Scan > ScanState::Idle) {
                s_MainProcess = MainProcess::WaitF_3;
            } else {
                s_MainProcess = MainProcess::Review_Rack;
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

        /* 4.Review All Sample */
    case MainProcess::Review_Rack:
        /* Pass-over */
        s_MainProcess = MainProcess::Recycle_Rack;
        break;

#ifdef Debug_Main
        qDebug() << mUserId << "Main 4 Review Rack";
#endif
        if (s_ReviewRack == ReviewRackState::Finish) {
            s_MainProcess = MainProcess::Recycle_Rack;
        } else {
            ReviewRack();
            if (s_ReviewRack > ReviewRackState::Idle) {
                s_MainProcess = MainProcess::WaitF_4;
            } else {
                s_MainProcess = MainProcess::Recycle_Rack;
            }
        }
        break;
    case MainProcess::WaitF_4:
        if (s_ReviewRack > ReviewRackState::Idle) {
            ReviewRack();
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;

        /* 5.Recycle Rack */
    case MainProcess::Recycle_Rack:
#ifdef Debug_Main
        qDebug() << mUserId << "Main 5 Recycle Rack";
#endif
        RecycleRack();
        if (s_RecycleRack == RecycleRackState::Finish) {
            s_MainProcess = MainProcess::Move_To_Last_Postion;
        } else if (s_RecycleRack > RecycleRackState::Idle) {
            s_MainProcess = MainProcess::WaitF_5;
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;
    case MainProcess::WaitF_5:
        if (mLock == true) {
            if (s_RecycleRack > RecycleRackState::Move_To_Export_Pos) {
                RecycleRack();
            } else {
                s_MainProcess = MainProcess::Idle;
            }
        } else {
            if (s_RecycleRack > RecycleRackState::Idle) {
                RecycleRack();
                if (s_RecycleRack == RecycleRackState::Finish) {
                    if (mCoord.pos > Pos_Exit_E1) {
                        s_MainProcess = MainProcess::Reset;
                    } else {
                        s_MainProcess = MainProcess::Move_To_Last_Postion;
                    }
                }
            }
        }
        break;
//! [5.Recycle Rack]

    case MainProcess::Move_To_Last_Postion:
    {
#ifdef Debug_Main
        qDebug() << mUserId << "Main 6 Move_To_Last_Station";
#endif
        bool ok = req_PosSxScan("S2", 0);
        if (ok) {
            s_MainProcess = MainProcess::WaitF_Last_Postion;
        } else {
            s_MainProcess = MainProcess::Idle;
        }
    }
        break;
    case MainProcess::WaitF_Last_Postion:
        if (isCmd_PosScan_done("S2") == true) {
            s_MainProcess = MainProcess::Reset;
        }
        break;

    case MainProcess::Reset:
    {
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

    case ScanState::Scan_RackCode_Idle:
        if (m_isScanRackCodeFinished == false) {
            logProcess("Scan", 1, "Scan Rack Code");
            s_Scan = ScanState::Move_To_RackScan_Pos;
        } else {
            s_Scan = ScanState::Idle;
        }
        break;

    case ScanState::Move_To_RackScan_Pos:
    {
        bool ok = req_PosSxScanRackCode("S1");
        if (ok) {
            s_Scan = ScanState::WaitF_Pos_RackScan_Done;
        }
    }
        break;
    case ScanState::WaitF_Pos_RackScan_Done:
        if (isCmd_PosScanRackCode_done("S1")) {
            s_Scan = ScanState::Scan_RackCode;
        }
        break;

    case ScanState::Scan_RackCode:
        cmd_Scan_Open(1, 1000);
        s_Scan = ScanState::WaitF_Scan_RackCode_Done;
        break;
    case ScanState::WaitF_Scan_RackCode_Done:
        if (isScanOpen_Done(1) && m_isScanRackCodeFinished) {
            mRack->setBarcode(m_scanRackBarCode);
            if (isSmearOnly(m_scanRackBarCode)) {
                mRack->setSmearMode();
            }
            s_Scan = ScanState::Idle;
        }
        break;

    case ScanState::Idle:
    {
        int pos = mRack->getNextScanPos();
        if (pos >= 0) {
            mScan_pos = pos;
            logProcess("Scan", 1, "Idle", QString("pos:%1").arg(mScan_pos));
            s_Scan = ScanState::Get_Scan_Station;
        } else {
            s_Scan = ScanState::Finish;
        }
        break;
    }

    case ScanState::Get_Scan_Station:
    {
        auto station = mStatMgr->getScanStation();
        if (station != nullptr) {
            mScanStation = station;
            logProcess("Scan", 1, "Get_Scan_Station", QString("pos:%1 station:%2").arg(mScan_pos).arg(mScanStation->mid()));
            s_Scan = ScanState::Move_To_Scan_Pos;
        }
        break;
    }

    case ScanState::Move_To_Scan_Pos:
    {
        bool ok = req_PosSxScan(mScanStation->userid(), mScan_pos);
        if (ok) {
            logProcess("Scan", 2, "Move_To_Scan_Pos",
                       QString("rack pos:%1, station:%2").arg(mScan_pos).arg(mScanStation->mid()));
            s_Scan = ScanState::WaitF_Pos_Done;
        }
        break;
    }
    case ScanState::WaitF_Pos_Done:
        if (isCmd_PosScan_done(mScanStation->userid())) {
            logProcess("Scan", 3, "WaitF_Pos_Done");
            s_Scan = ScanState::Send_Station_Scan_Request;
        }
        break;

    case ScanState::Send_Station_Scan_Request:
        mScanStation->scanSampleBarcode(mRack, mScan_pos);
        logProcess("Scan", 4, "Send_Station_Scan_Request", "ask station to scan sample");
        s_Scan = ScanState::WaitF_Station_Scan_Done;
        break;
    case ScanState::WaitF_Station_Scan_Done:
        if (mScanStation->isScanFinished()) {
            logProcess("Scan", 5, "Scan Finished", QString("pos:%1").arg(mScan_pos));
            s_Scan = ScanState::Done;
        }
        break;

    case ScanState::Done:
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
        mUnsendSampleList = mRack->getUnSendToTestSampleList();
        if (mUnsendSampleList.isEmpty() == false) {
            index = 0;
            s_Send = SendState::Get_Untest_Sample;
        }
        break;

    case SendState::Get_Untest_Sample:
    {
        if (index >= 0 && index < mUnsendSampleList.count()) {
            mTodoSample = mUnsendSampleList.at(index);
            s_Send = SendState::Get_Matched_Station;
        } else {
            s_Send = SendState::Idle;
        }
    }
        break;
    case SendState::Get_Matched_Station:
    {
        bool ok = mStatMgr->reqSendStation(mTodoSample);
        if (ok) {
            s_Send = SendState::WaitF_Station_Response;
        }
    }
        break;

    case SendState::WaitF_Station_Response:
    {
        bool ok = mStatMgr->isRequestTestStationFinished();
        if (ok) {
            auto station = mStatMgr->getDestinationStation(mTodoSample->sid());
            if (station) {
                mSendSample = mTodoSample;
                mSendStation = station;
                mTodoSample = nullptr;

                logProcess("Send", 1, "Send", QString("[Rack:%1-%2] -> [%3]")
                           .arg(mRack->rackid()).arg(mSendSample->rack_pos()).arg(mSendStation->mid()));
                s_Send = SendState::Move_To_Test_Pos;
            } else {
                s_Send = SendState::Idle;
#if 0
                index++;        // try another one
                index = 99;     // one by one test in line
                s_Send = SendState::Get_Untest_Sample;
#endif
            }
        }
    }
        break;

    case SendState::Move_To_Test_Pos:
    {
        bool ok = req_PosSxTest(mSendStation->userid(), mSendSample->rack_pos());
        if (ok) {
            s_Send = SendState::WaitF_Test_Pos_Done;
        }
        break;
    }
    case SendState::WaitF_Test_Pos_Done:
        if (isCmd_PosTest_done(mSendStation->userid())) {
            logProcess("Send", 2, "Arrive_Station", QString("[Rack:%1-%2] -- [%3]")
                       .arg(mRack->rackid()).arg(mSendSample->rack_pos()).arg(mSendStation->mid()));
            s_Send = SendState::Send_New_Sample_To_Station;
        }
        break;

    case SendState::Send_New_Sample_To_Station:
    {
        bool ok = mSendStation->receiveNewSample(mSendSample);
        if (ok) {
            RtSampleManager::GetInstance()->sendSamplePath(mSendSample, mSendStation->userid());
            logProcess("Send", 3, "Send_New_Sample_To_Station", QString("sample:%1").arg(mSendSample->sid()));
            s_Send = SendState::WaitF_Send_Sample_To_Station_Done;
        }
        break;
    }

    case SendState::WaitF_Send_Sample_To_Station_Done:
        if (mSendStation->receiveSampleFinish()) {
            mSendSample->setStation(mSendStation);
            s_Send = SendState::Done;
        }
        break;
    case SendState::Done:
        logProcess("Send", 99, "Done", mSendSample->sid());
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
            mRecycleStation = sample->getStation();
            logProcess("Recycle Sample", 1, "Move_To_Station_Pos", QString("recycle sample:%1 from:%2")
                       .arg(mRecycleSample->sid(), mRecycleStation->mid()));
            s_RecycleSample = RecycleSampleState::Move_To_Station_Pos;
        }
        break;
    }

    case RecycleSampleState::Move_To_Station_Pos:
    {
        bool ok = req_PosSxTest(mRecycleStation->userid(), mRecycleSample->rack_pos());
        if (ok) {
            s_RecycleSample = RecycleSampleState::WaitF_Move_To_Station_Pos;
        }
        break;
    }
    case RecycleSampleState::WaitF_Move_To_Station_Pos:
        if (isCmd_PosTest_done(mRecycleStation->userid())) {
            logProcess("Recycle Sample", 2, "Send_Recycle_Sample");
            s_RecycleSample = RecycleSampleState::Send_Recycle_Sample;
        }
        break;

    case RecycleSampleState::Send_Recycle_Sample:
    {
        bool ok = mRecycleStation->recycleSample(mRecycleSample);
        if (ok) {
            s_RecycleSample = RecycleSampleState::WaitF_Recyle_Sample_Done;
        }
        break;
    }
    case RecycleSampleState::WaitF_Recyle_Sample_Done:
        if (mRecycleStation->isRecycleSampleFinish()) {
            s_RecycleSample = RecycleSampleState::Done;
        }
        break;

    case RecycleSampleState::Done:
        logProcess("Recycle Sample", 99, "Done", mRecycleSample->sid());
        mRecycleSample = nullptr;

        if (mRack->isAllFinished()) {
            s_RecycleSample = RecycleSampleState::Finish;
        } else {
            s_RecycleSample = RecycleSampleState::Idle;
        }
        break;
    }
}

//! [4.Review Sample Diagnose]
void MCart::ReviewRack()
{
    switch (s_ReviewRack) {
    case ReviewRackState::Finish:
        break;

    case ReviewRackState::Idle:
        if (mRack->isAllReviewedDone()) {
            logProcess("Review", 99, "Finish");
            s_ReviewRack = ReviewRackState::Finish;
        } else if (FMessageCenter::GetInstance()->isDMUConnected()) {
            if (mRack->isAllTestFinished() && mRack->isAllReviewed() == false) {
                if (mRack->isSmearMode()) {
                    mRack->setReviewMode(true, "smear");
                }
                logProcess("Review", 0, "Start");
                s_ReviewRack = ReviewRackState::Move_To_Review_Pos;
            }
        }
        break;

    case ReviewRackState::Move_To_Review_Pos:
    {
        bool ok = req_PosSxScan("S2", 0);
        if (ok) {
            s_ReviewRack = ReviewRackState::WaitF_Review_Pos_Done;
        }
    }
        break;
    case ReviewRackState::WaitF_Review_Pos_Done:
        if (isCmd_PosScan_done("S2") == true) {
            m_reviewTimeoutTimer->start();
            m_isReviewTimeout = false;
            s_ReviewRack = ReviewRackState::WaitF_Review_Result_Done;
        }
        break;
    case ReviewRackState::WaitF_Review_Result_Done:
        if (mRack->isAllReviewed() || m_isReviewTimeout == true) {
            m_reviewTimeoutTimer->stop();
            bool isClassify = mStatMgr->mS1->isConnected() && mStatMgr->mS1->isUnited();
            bool isSmear = mStatMgr->mSmear->isConnected() && mStatMgr->mSmear->isUnited();
            mRack->doReview(isClassify, isSmear);

            logProcess("Review", 99, "Done");
            s_ReviewRack = ReviewRackState::Done;
        }
        break;
    case ReviewRackState::Done:
        s_ReviewRack = ReviewRackState::Idle;
        break;
    }
}

//! [5.Recycle Rack]
void MCart::RecycleRack()
{
    switch (s_RecycleRack) {
    case RecycleRackState::Finish:
        break;
    case RecycleRackState::Idle:
        if (mRack->isCanceled() || mRack->isAllFinished()) {
            logProcess("Recycle Rack", 0, "Start");
            s_RecycleRack = RecycleRackState::Get_Export;
        }
        break;

    case RecycleRackState::Get_Export:
        if (mRack->isSampleSmearMode() == true && mE2->isEnable()) {
            mExport = mE2;
        } else {
            mExport = mE1;
        }
        logProcess("Recycle Rack", 1, "Get_Export", QString("recycle rack:%1 from %2")
                   .arg(mRack->rackid(), mExport->mid()));
        s_RecycleRack = RecycleRackState::Move_To_Export_Pos;
        break;

    case RecycleRackState::Move_To_Export_Pos:
    {
        bool ok = req_PosExitEx(mExport->userid());
        if (ok) {
            logProcess("Recycle Rack", 2, "Move_To_Export_Pos");
            s_RecycleRack = RecycleRackState::WaitF_Export_Pos_Done;
        }
        break;
    }
    case RecycleRackState::WaitF_Export_Pos_Done:
        if (isCmd_PosExit_Ex_done(mExport->userid()) == true) {
            m_timeoutTimer->stop();
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
    }
}

bool MCart::isSmearOnly(const QString &code)
{
    bool en = FCommon::getConfigFileValue("general", "smearOnly", "enable").toBool();
    if (en == true) {
        QString barcode = FCommon::getConfigFileValue("general", "smearOnly", "barcode").toString();
        if (barcode == code) {
            return true;
        }
    }
    return false;
}

void MCart::cancelTimeoutTimer_slot()
{
    if (mRelatePos == RePos_Front || mRelatePos == RePos_Single) {
        if (mCoord.pos >= Pos_Origin && m_isLocked == false) {
            m_timeoutCount++;
            qDebug() << mUserId << "Waiting:" << m_timeoutCount;
            if (m_timeoutCount >= m_MaxWaiting) {
                mRack->setCanceled();
                m_timeoutTimer->stop();
                if (s_MainProcess < MainProcess::Recycle_Rack) {
                    s_MainProcess = MainProcess::Recycle_Rack;
                }
            }
        }
    }
}

void MCart::onReviewTimeout_slot()
{
    m_isReviewTimeout = true;
    qDebug() << "review timeout.";
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
    } else if (api.contains("Scan_Open")) {
        if (mCoord.pos == Pos_Scan_RackCode_1) {
            m_scanRackBarCode.clear();
            if (resValue.isObject()) {
                QJsonObject obj = resValue.toObject();
                m_scanRackBarCode = obj.value("string").toString().simplified();
            }
            m_isScanRackCodeFinished = true;
        }
    }
}
