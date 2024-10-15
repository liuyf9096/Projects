#include "m_cart.h"
#include "rack/rt_rack_manager.h"
#include "carts_manager.h"
#include "station/station_manager.h"
#include "module_manager.h"
#include "exit/m_export1.h"
#include "exit/m_export2.h"
#include "settings/f_settings.h"

#include <QDateTime>

QMap<QString, int> CartCoord::CoordBaseMap = {
    {"Reset", 0},

    {"PosImport", 10},

    {"PosS1Scan", 12},
    {"PosS1Test", 12 + 2},

    {"PosExit_E1", 27},

    {"PosS2Scan", 29},
    {"PosS2Test", 29 +2},

    {"PosS3Scan", 50},
    {"PosS3Test", 50 + 2},

    {"PosExit_E2", 66}
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
    mStats = StationManager::GetInstance();

    m_isEnable = true;
    mCoord.setName(userid);

    mMainTimer = new QTimer(this);
    mMainTimer->setInterval(50);
    connect(mMainTimer, &QTimer::timeout, this, &MCart::processTimer_slot);

    m_WaitTimer = new QTimer(this);
    m_WaitTimer->setInterval(5000);
    m_WaitTimer->setSingleShot(true);
    connect(m_WaitTimer, &QTimer::timeout, this, &MCart::waitTimer_slot);

    mRack = nullptr;
    state_init();

    auto track2 = RtDeviceManager::GetInstance()->track2();
    connect(track2, &DTrack2::onFunctionFinished_signal,
            this, &MCart::onFunctionFinished_slot);
}

void MCart::state_init()
{
    mLock = false;
    m_isLocked = false;
    m_isResetting = false;

    s_MainProcess = MainProcess::Idle;
    s_Scan = ScanState::Idle;
    s_Send = SendState::Idle;
    s_RecycleSample = RecycleSampleState::Idle;
    s_ReviewRack = ReviewRackState::Idle;
    s_RecycleRack = RecycleRackState::Idle;

    mScanStation = nullptr;
    mTodoList.clear();
    mTodoSample = nullptr;
    mSendSample = nullptr;
    mSendStation = nullptr;

    mRecycleSample = nullptr;
    mRecycleStation = nullptr;

    mExport = nullptr;

    logProcess("Reset", 0, "state_init");
}

void MCart::start()
{
    mMainTimer->start();
    DModuleBase::start();
}

void MCart::reset()
{
    state_init();

    DModuleBase::reset();
}

void MCart::stop()
{
    mMainTimer->stop();
    DModuleBase::stop();
}

bool MCart::permitAlliedBack()
{
    if (mRack == nullptr) {
        return true;
    } else if (mRack->getUnTestedPos() > 8 && mCoord.coord > 16) {
        return true;
    }
    return false;
}

void MCart::cmd_Reset()
{
    if (CartCoord::CoordBaseMap.contains("Reset") == false) {
        qFatal("Can NOT find coord.");
    }
    m_isResetting = true;
    mCoord.toCoord = CartCoord::CoordBaseMap.value("Reset");
}

void MCart::cmd_PosImport()
{
    if (CartCoord::CoordBaseMap.contains("PosImport") == false) {
        qFatal("Can NOT find coord.");
    }
    int to = CartCoord::CoordBaseMap.value("PosImport");
    Q_ASSERT(to > 0);
    mCoord.toCoord = to;
}

void MCart::cmd_PosSxScan(const QString &api, int pos)
{
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosSxScan coord.");
    }
    int to = CartCoord::CoordBaseMap.value(api) + pos;
    Q_ASSERT(to > pos);
    mCoord.toCoord = to;
}

void MCart::cmd_PosSxTest(const QString &api, int pos)
{
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosSxTest coord.");
    }
    int to = CartCoord::CoordBaseMap.value(api) + pos;
    Q_ASSERT(to > pos);
    mCoord.toCoord = to;
}

void MCart::cmd_PosExit_Ex(const QString &api)
{
    if (CartCoord::CoordBaseMap.contains(api) == false) {
        qDebug() << "pos:" << api;
        qFatal("Can NOT find PosExit_Ex coord.");
    }
    int to = CartCoord::CoordBaseMap.value(api);
    Q_ASSERT(to > 0);
    mCoord.toCoord = to;
}

void MCart::cmd_PosLeft(int offset)
{
    mCoord.toCoord = mCoord.coord + offset;
}

void MCart::cmd_PosRight(int offset)
{
    mCoord.toCoord = mCoord.coord - offset;
}

void MCart::newRack()
{
    if (mRack == nullptr && mCoord.pos == Pos_Import) {
        mRack = RtRackManager::GetInstance()->NewRackObj();
        mRack->setCartId(mid());

        state_init();
        mMainTimer->start();
    }
}

void MCart::onFunctionFinished_slot(const QString &api, const QJsonValue &)
{
    if (api.startsWith(mUserId)) {
        updatePosition(api);
    } else if (api == "Reset") {
        m_isResetting = false;
        mCoord.pos = Pos_Origin;
        mCoord.setCoord(0);
    }
}

//! [Main]
void MCart::processTimer_slot()
{
    if (dev->track1()->isResetOk() == false) {
        return;
    }

    switch (s_MainProcess) {
    case MainProcess::Idle:
        if (mRack != nullptr) {
#ifdef Q_OS_WIN
            qDebug() << "Main  0 Idle";
#endif
            if (mLock == true) {
                m_isLocked = true;
                logProcess("Main", -1, "Lock");
                s_MainProcess = MainProcess::Wait_Another_Cart;
            } else {
                s_MainProcess = MainProcess::Recycle_Sample;
            }
        }
        break;

    case MainProcess::Wait_Another_Cart:
        if (mLock == false) {
            logProcess("Main", -1, "Unlock");
            m_isLocked = false;
            s_MainProcess = MainProcess::Recycle_Sample;
        }
        break;

//! [1.Recycle Sample]
    case MainProcess::Recycle_Sample:
        if (s_RecycleSample == RecycleSampleState::Finish) {
            s_MainProcess = MainProcess::Send_Sample;
        } else {
#ifdef Q_OS_WIN
            qDebug() << mUserId << "Main 1 Recycle Sample";
#endif
            RecycleSampleFrStation();
            if (s_RecycleSample > RecycleSampleState::Idle) {
                s_MainProcess = MainProcess::WaitF_1;
            } else {
                s_MainProcess = MainProcess::Send_Sample;
            }
        }
        break;
    case MainProcess::WaitF_1:
        if (s_RecycleSample > RecycleSampleState::Idle && mLock == false) {
            RecycleSampleFrStation();
        } else {
            s_MainProcess = MainProcess::Send_Sample;
        }
        break;
//! [1.Recycle Sample]

//! [2.Send Sample]
    case MainProcess::Send_Sample:
        if (s_Send == SendState::Finish) {
            s_MainProcess = MainProcess::Scan_Sample;
        } else {
#ifdef Q_OS_WIN
            qDebug() << mUserId << "Main 2 Send Sample";
#endif
            SendToStatProcess();
            if (s_Send >= SendState::Move_To_Staion_Pos) {
                s_MainProcess = MainProcess::WaitF_2;
            } else if (s_Send == SendState::Idle) {
                s_MainProcess = MainProcess::Scan_Sample;
            }
        }
        break;
    case MainProcess::WaitF_2:
        if (s_Send > SendState::Idle && mLock == false) {
            SendToStatProcess();
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;
//! [2.Send Sample]

//! [3.Scan Sample]
    case MainProcess::Scan_Sample:
        if (s_Scan == ScanState::Finish) {
            s_MainProcess = MainProcess::Review_Rack;
        } else {
#ifdef Q_OS_WIN
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
        if (s_Scan > ScanState::Idle && mLock == false) {
            ScanBarcodeProcess();
        } else {
            s_MainProcess = MainProcess::Review_Rack;
        }
        break;
//! [3.Scan Sample]

//! [4.Review Rack]
    case MainProcess::Review_Rack:
#ifdef Q_OS_WIN
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
        if (s_ReviewRack != ReviewRackState::Finish) {
            ReviewRack();
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;
//! [4.Review Rack]

//! [5.Recycle Rack]
    case MainProcess::Recycle_Rack:
#ifdef Q_OS_WIN
        qDebug() << mUserId << "Main  5 Recycle Rack";
#endif
        RecycleRack();
        if (s_RecycleRack > RecycleRackState::Idle) {
            s_MainProcess = MainProcess::WaitF_5;
        } else {
            s_MainProcess = MainProcess::Idle;
        }
        break;
    case MainProcess::WaitF_5:
        if (s_RecycleRack > RecycleRackState::Idle) {
            RecycleRack();
            if (s_RecycleRack == RecycleRackState::Finish && mLock == false) {
                if (mCoord.pos > Pos_Exit_E1) {
                    s_MainProcess = MainProcess::Reset;
                } else {
                    s_MainProcess = MainProcess::Move_To_Last_Postion;
                }
            }
        }
        break;
//! [5.Recycle Rack]

    case MainProcess::Move_To_Last_Postion:
    {
#ifdef Q_OS_WIN
        qDebug() << mUserId << "Main 6 Move_To_Last_Station";
#endif
        bool ok = req_PosSxScan("S3", 0);
        if (ok) {
            s_MainProcess = MainProcess::WaitF_Last_Postion;
        }
    }
        break;
    case MainProcess::WaitF_Last_Postion:
        if (isCmd_PosScan_done("S3") == true) {
            s_MainProcess = MainProcess::Reset;
        }
        break;

    case MainProcess::Reset:
    {
        bool ok = req_Reset();
        if (ok) {
            logProcess("Main", 99, "All_Finish");
#ifdef Q_OS_WIN
            qDebug() << mUserId << "Main 7 All_Finish";
#endif
            mRack = nullptr;

            mMainTimer->stop();
            s_MainProcess = MainProcess::All_Finish;
        }
    }
        break;

    case MainProcess::All_Finish:
        break;
    }
}

//! [1.Scan]
void MCart::ScanBarcodeProcess()
{
    switch (s_Scan) {
    case ScanState::Finish:
        break;

    case ScanState::Idle:
    {
        int pos = mRack->getNextScanPos();
        if (pos >= 0) {
            mScanPos = pos;
            logProcess("Scan", 0, "Idle", QString("pos:%1").arg(mScanPos));
            s_Scan = ScanState::Get_Scan_Station;
        } else {
            s_Scan = ScanState::Finish;
        }
        break;
    }

    case ScanState::Get_Scan_Station:
    {
        auto station = mStats->getScanStation();
        if (station != nullptr) {
            mScanStation = station;
            logProcess("Scan", 1, "Get_Scan_Station", QString("pos:%1 station:%2").arg(mScanPos).arg(mScanStation->mid()));
            s_Scan = ScanState::Move_To_Scan_Pos;
        }
        break;
    }

    case ScanState::Move_To_Scan_Pos:
    {
        bool ok = req_PosSxScan(mScanStation->userid(), mScanPos);
        if (ok) {
            logProcess("Scan", 2, "Move_To_Scan_Pos",
                       QString("rack pos:%1, station:%2").arg(mScanPos).arg(mScanStation->mid()));
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
        mScanStation->scanSampleBarcode(mRack, mScanPos);
        logProcess("Scan", 4, "Send_Station_Scan_Request", "ask station to scan sample");
        s_Scan = ScanState::WaitF_Station_Scan_Done;
        break;
    case ScanState::WaitF_Station_Scan_Done:
        if (mScanStation->isScanFinished()) {
            logProcess("Scan", 5, "WaitF_Station_Scan_Done");
            s_Scan = ScanState::Done;
        }
        break;

    case ScanState::Done:
        logProcess("Scan", 99, "Done", QString("pos:%1").arg(mScanPos));
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
    {
        if (mStats->mS1->isUnited() || mStats->mS2->isUnited()) {
            mTodoList = mRack->getUnSendToTestList();
            if (!mTodoList.isEmpty()) {
                index = 0;
                logProcess("Send", 0, "Send", "Unsend to Test");
                s_Send = SendState::Get_Untest_Sample;
            } else {
                mTodoList = mRack->getUnSendToSmearList();
                if (!mTodoList.isEmpty()) {
                    index = 0;
                    logProcess("Send", 0, "Send", "Unsend to Smear");
                    s_Send = SendState::Get_UnSmear_Sample;
                }
            }
        } else if (mStats->mS3->isUnited()) {
            /* preprocessing */
            mRack->setAllSampleReviewed();
            mRack->setAllSmearProgram();

            mTodoList = mRack->getUnSendToSmearList();
            if (!mTodoList.isEmpty()) {
                index = 0;
                logProcess("Send", 0, "Send", "Unsend to Smear");
                s_Send = SendState::Get_UnSmear_Sample;
            }
        }
        break;
    }

//! [Test]
    case SendState::Get_Untest_Sample:
    {
        if (index >= 0 && index < mTodoList.count()) {
            mTodoSample = mTodoList.at(index);
            s_Send = SendState::Get_Test_Station;
        } else {
            s_Send = SendState::Idle;
        }
    }
        break;
    case SendState::Get_Test_Station:
    {
        bool ok = mStats->requestTestStation(mTodoSample, mCoord.coord);
        if (ok) {
            s_Send = SendState::WaitF_Test_Station_Done;
        }
    }
        break;

    case SendState::WaitF_Test_Station_Done:
    {
        bool ok = mStats->isRequestTestStationFinished();
        if (ok) {
            auto station = mStats->getDestinationStation(mTodoSample->sid());
            if (station) {
                mSendSample = mTodoSample;
                mSendStation = station;
                mTodoSample = nullptr;

                logProcess("Send", 1, "Send", QString("[Rack:%1-%2] -> [%3]")
                           .arg(mRack->rackid()).arg(mSendSample->rack_pos()).arg(mSendStation->mid()));
                s_Send = SendState::Move_To_Staion_Pos;
            } else {
                index++;

                index = 99;//for test
                qDebug() << "todo" << index;
                s_Send = SendState::Get_Untest_Sample;
            }
        }
    }
        break;
//! [Test]
//! [Smear]
    case SendState::Get_UnSmear_Sample:
        if (index >= 0 && index < mTodoList.count()) {
            mTodoSample = mTodoList.at(index);
            s_Send = SendState::Request_Smear_Station;
        } else {
            s_Send = SendState::Idle;
        }
        break;

    case SendState::Request_Smear_Station:
    {
        bool ok = mStats->requestSmearStation(mTodoSample);
        if (ok) {
            s_Send = SendState::WaitF_Smear_Station_Done;
        }
        break;
    }
    case SendState::WaitF_Smear_Station_Done:
    {
        bool ok = mStats->isRequestTestStationFinished();
        if (ok) {
            auto station = mStats->getDestinationStation(mTodoSample->sid());
            if (station) {
                mSendSample = mTodoSample;
                mSendStation = station;

                logProcess("Send", 1, "Send", QString("[Rack:%1-%2] -> [%3]")
                           .arg(mRack->rackid()).arg(mSendSample->rack_pos()).arg(mSendStation->mid()));
                s_Send = SendState::Move_To_Staion_Pos;
            } else {
                index++;
                index = 99;//for test
                qDebug() << "todo" << index;
                s_Send = SendState::Get_UnSmear_Sample;
            }
        }
    }
        break;
//! [Smear]

    case SendState::Move_To_Staion_Pos:
    {
        bool ok = req_PosSxTest(mSendStation->userid(), mSendSample->rack_pos());
        if (ok) {

            JPacket p(PacketType::Notification);
            p.device = "track";
            p.module = "sample";
            p.api = "SamplePath";

            QJsonObject obj;
            obj.insert("sample_id", mSendSample->sampleSN());
            obj.insert("station_id", mSendStation->userid());
            obj.insert("timestamp", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
            p.paramsValue = obj;
            sendUIMessage(p);

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
        logProcess("Send", 99, "Finish", mSendSample->sid());
        mSendSample = nullptr;
        s_Send = SendState::Idle;
        break;
    }
}

//! [3.Recycle Sample From Station]
void MCart::RecycleSampleFrStation()
{
    switch (s_RecycleSample) {
    case RecycleSampleState::Finish:
        break;

    case RecycleSampleState::Idle:
    {
        auto sample = mRack->getSamplingFinishedSample();
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
        logProcess("Recycle Sample", 99, "Finish", mRecycleSample->sid());
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
    case ReviewRackState::Idle:
        if (mRack->isNeedReview()) {
            mRack->setAllSampleReviewed();
            if (FMessageCenter::GetInstance()->isIPUConnected()) {
                logProcess("Review", 0, "Start");
                s_ReviewRack = ReviewRackState::Move_To_Review_Pos;
            } else {
                s_ReviewRack = ReviewRackState::Finish;
            }
        }
        break;

    case ReviewRackState::Move_To_Review_Pos:
    {
        bool ok = req_PosSxScan("S3", 0);
        if (ok) {
            s_ReviewRack = ReviewRackState::WaitF_Review_Pos_Done;
        }
    }
        break;
    case ReviewRackState::WaitF_Review_Pos_Done:
        if (isCmd_PosScan_done("S3") == true) {
            m_WaitTimer->start();
            m_waitFinished = false;
            s_ReviewRack = ReviewRackState::WaitF_Review_Result_Done;
        }
        break;
    case ReviewRackState::WaitF_Review_Result_Done:
        if (m_waitFinished == true) {
            auto smear = mStats->mS3;
            if (smear->isConnected() && smear->isUnited() && !mRack->isEmpty()) {
                if (mRack->rack_index() % 2 == 0) {
                    if (FSettings::GetInstance()->isSmearAll()) {
                        mRack->setAllSmearProgram();
                    } else {
                        mRack->setTestHalfSmearProgram();
                    }
                }
            }
            logProcess("Review", 99, "Finish");
            s_ReviewRack = ReviewRackState::Finish;
        }
        break;
    case ReviewRackState::Finish:
        break;
    }
}

//! [5.Recycle Rack]
void MCart::RecycleRack()
{
    switch (s_RecycleRack) {
    case RecycleRackState::Idle:
        if (mRack->isAllFinished()) {
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

void MCart::waitTimer_slot()
{
    m_waitFinished = true;
}
