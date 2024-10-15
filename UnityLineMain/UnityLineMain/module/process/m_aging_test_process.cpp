#include "m_aging_test_process.h"
#include "cart/carts_manager.h"
#include "station/station_manager.h"
#include "f_common.h"

MAgingTestProcess::MAgingTestProcess(QObject *parent)
    : DModuleBase("agingtest", "agingtest", parent)
    , m_count(0)
    , m_closet1_count(0)
    , m_closet2_count(0)
{
    state_init();

    if (mRecordDb) {
        auto arr = mRecordDb->selectRecord("aging", "project='main'");
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            m_count = obj.value("count").toInt();
        }
        arr = mRecordDb->selectRecord("aging", "project='closet1'");
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            m_closet1_count = obj.value("count").toInt();
        }
        arr = mRecordDb->selectRecord("aging", "project='closet2'");
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            m_closet2_count = obj.value("count").toInt();
        }
    }

    m_agingTimer = new QTimer(this);
    m_agingTimer->setInterval(100);
    connect(m_agingTimer, &QTimer::timeout,
            this, &MAgingTestProcess::onAgingTimer_slot);
}

void MAgingTestProcess::state_init()
{
    m_index = 0;
    s_aging = TrackState::Idle;
    s_closet1 = ClosetState::Idle;
    s_closet2 = ClosetState::Idle;
}

void MAgingTestProcess::start()
{
    m_agingTimer->start();
    DModuleBase::start();
}

void MAgingTestProcess::reset()
{
    state_init();
    DModuleBase::reset();
}

void MAgingTestProcess::stop()
{
    m_agingTimer->stop();
    DModuleBase::stop();
}

void MAgingTestProcess::onAgingTimer_slot()
{
    trackProcess();
    closet1Process();
    closet2Process();
}

void MAgingTestProcess::trackProcess()
{
    switch (s_aging) {
    case TrackState::Idle:
        s_aging = TrackState::Check_New_Rack;
        break;

    case TrackState::Check_New_Rack:
        dev->track()->cmd_CheckSensorValue();
        s_aging = TrackState::WaitF_Check_Done;
        break;
    case TrackState::WaitF_Check_Done:
        if (dev->track()->isFuncDone("CheckSensorValue")) {
            if (dev->track()->checkSensorValue("Import_new_rack") == true) {
                auto cart = CartsManager::GetInstance()->getSpareCart();
                if (cart) {
                    mCart = cart;
                    s_aging = TrackState::Move_To_Import;
                }
            } else {
                s_aging = TrackState::Reset_Carts;
            }
        }
        break;

    case TrackState::Move_To_Import:
    {
        bool ok = mCart->req_PosImport();
        if (ok) {
           s_aging = TrackState::WaitF_Import_Pos_Done;
        }
        break;
    }
    case TrackState::WaitF_Import_Pos_Done:
        if (mCart->isCmd_PosImport_done()) {
            s_aging = TrackState::Load_Rack;
        }
        break;

    case TrackState::Load_Rack:
        dev->track()->cmd_Import_Load();
        s_aging = TrackState::WaitF_Load_Rack_Done;
        break;
    case TrackState::WaitF_Load_Rack_Done:
        if (dev->track()->isFuncDone("Import_Load")) {
            dev->track()->cmd_Import_Reset();
            s_aging = TrackState::Move_To_Scan1_Pos;
        }
        break;

    case TrackState::Move_To_Scan1_Pos:
        mCart->req_PosSxScan("S1", m_index);
        s_aging = TrackState::WaitF_Scan1_Pos_Done;
        break;
    case TrackState::WaitF_Scan1_Pos_Done:
        if (mCart->isCmd_PosScan_done("S1")) {
            s_aging = TrackState::Open_Scan1;
        }
        break;

    case TrackState::Open_Scan1:
        dev->track()->cmd_S1_Scan_Open(500);
        s_aging = TrackState::WaitF_Open_Scan1_Done;
        break;
    case TrackState::WaitF_Open_Scan1_Done:
        if (dev->track()->isFuncDone("S1_Scan_Open")) {
            m_index++;
            if (m_index < 5) {
                s_aging = TrackState::Move_To_Scan1_Pos;
            } else {
                s_aging = TrackState::Move_To_Scan2_Pos;
            }
        }
        break;

    case TrackState::Move_To_Test1_Pos:
        mCart->req_PosSxTest("S1", m_index);
        s_aging = TrackState::WaitF_Test1_Done;
        break;
    case TrackState::WaitF_Test1_Done:
        if (mCart->isCmd_PosTest_done("S1")) {
            s_aging = TrackState::Move_To_Scan2_Pos;
        }
        break;

    case TrackState::Move_To_Scan2_Pos:
        if (FCommon::GetInstance()->stationCount() > 1) {
            mCart->req_PosSxScan("S2", m_index);
            s_aging = TrackState::WaitF_Scan2_Pos_Done;
        } else {
            s_aging = TrackState::Move_To_Export_Pos;
        }
        break;
    case TrackState::WaitF_Scan2_Pos_Done:
        if (mCart->isCmd_PosScan_done("S2")) {
            s_aging = TrackState::Open_Scan2;
        }
        break;

    case TrackState::Open_Scan2:
        dev->track()->cmd_S2_Scan_Open(500);
        s_aging = TrackState::WaitF_Open_Scan2_Done;
        break;
    case TrackState::WaitF_Open_Scan2_Done:
        if (dev->track()->isFuncDone("S2_Scan_Open")) {
            m_index++;
            if (m_index < 10) {
                s_aging = TrackState::Move_To_Scan2_Pos;
            } else {
                s_aging = TrackState::Move_To_Scan3_Pos;
            }
        }
        break;

    case TrackState::Move_To_Test2_Pos:
        mCart->req_PosSxTest("S2", m_index);
        s_aging = TrackState::WaitF_Test2_Done;
        break;
    case TrackState::WaitF_Test2_Done:
        if (mCart->isCmd_PosTest_done("S2")) {
            s_aging = TrackState::Move_To_Scan3_Pos;
        }
        break;

    case TrackState::Move_To_Scan3_Pos:
        if (FCommon::GetInstance()->stationCount() > 2) {
            mCart->req_PosSxScan("S3", m_index);
            s_aging = TrackState::WaitF_Scan3_Pos_Done;
        } else {
            s_aging = TrackState::Move_To_Export_Pos;
        }
        break;
    case TrackState::WaitF_Scan3_Pos_Done:
        if (mCart->isCmd_PosScan_done("S3")) {
            s_aging = TrackState::Move_To_Test3_Pos;
        }
        break;

    case TrackState::Move_To_Test3_Pos:
        mCart->req_PosSxTest("S3", m_index);
        s_aging = TrackState::WaitF_Test3_Done;
        break;
    case TrackState::WaitF_Test3_Done:
        if (mCart->isCmd_PosTest_done("S3")) {
            s_aging = TrackState::Move_To_Export_Pos;
        }
        break;

    case TrackState::Move_To_Export_Pos:
        if (FCommon::GetInstance()->exitCount() == 1) {
            m_exitIndex = 1;
            bool ok = mCart->req_PosExitEx("E1");
            if (ok) {
                s_aging = TrackState::WaitF_Export_Done;
            }
        } else {
            bool ok = false;
            if (m_count % 2 == 0) {
                m_exitIndex = 1;
                ok = mCart->req_PosExitEx("E1");
            } else {
                m_exitIndex = 2;
                ok = mCart->req_PosExitEx("E2");
            }
            if (ok) {
                s_aging = TrackState::WaitF_Export_Done;
            }
        }
        break;
    case TrackState::WaitF_Export_Done:
        if (m_exitIndex == 1 && mCart->isCmd_PosExit_Ex_done("E1")) {
            s_aging = TrackState::UnLoad_Rack;
        } else if (m_exitIndex == 2 && mCart->isCmd_PosExit_Ex_done("E2")) {
            s_aging = TrackState::UnLoad_Rack;
        }
        break;

    case TrackState::UnLoad_Rack:
        if (m_exitIndex == 1) {
            dev->track()->cmd_Exit_E1_Unload();
        } else if (m_exitIndex == 2) {
            dev->track()->cmd_Exit_E2_Unload();
        }
        s_aging = TrackState::WaitF_UnLoad_Rack_Done;
        break;
    case TrackState::WaitF_UnLoad_Rack_Done:
        if (m_exitIndex == 1 && dev->track()->isFuncDone("Exit_E1_Unload")) {
            s_aging = TrackState::Reset_Cart;
        } else if (m_exitIndex == 2 && dev->track()->isFuncDone("Exit_E2_Unload")) {
            s_aging = TrackState::Reset_Cart;
        }
        break;

    case TrackState::Reset_Cart:
        m_count++;

        /* record */
        mRecordDb->updateRecord("aging", {{"count", m_count}}, {{"project", "main"}});

        mCart->req_Reset();
        s_aging = TrackState::Finish;
        break;
    case TrackState::WaitF_Reset_Cart_Done: //obsolete
        if (mCart->isCmd_Reset_done()) {
            s_aging = TrackState::Finish;
        }
        break;

    case TrackState::Reset_Carts:
    {
        bool ok = dev->track()->cmd_Reset();
        if (ok) {
            s_aging = TrackState::WaitF_Carts_Done;
        }
        break;
    }
    case TrackState::WaitF_Carts_Done:
        if (dev->track()->isFuncDone("Reset")) {
            s_aging = TrackState::Idle;
        }
        break;

    case TrackState::Finish:
        if (m_index >= 10) {
            m_index = 0;
        }
        s_aging = TrackState::Idle;
        break;

    default:
        break;
    }
}

void MAgingTestProcess::closet1Process()
{
    switch (s_closet1) {
    case ClosetState::Idle:
        s_closet1 = ClosetState::Open_Closet;
        break;

    case ClosetState::Open_Closet:
    {
        bool ok = dev->track()->cmd_S1_Emergency_Open();
        if (ok) {
            s_closet1 = ClosetState::WaitF_Closet_Open_Done;
        }
    }
        break;
    case ClosetState::WaitF_Closet_Open_Done:
        if (dev->track()->isFuncDone("S1_Emergency_Open")) {
            s_closet1 = ClosetState::Close_Closet;
        }
        break;

    case ClosetState::Close_Closet:
    {
        bool ok = dev->track()->cmd_S1_Emergency_Close();
        if (ok) {
            s_closet1 = ClosetState::WaitF_Closet_Close_Done;
        }
    }
        break;
    case ClosetState::WaitF_Closet_Close_Done:
        if (dev->track()->isFuncDone("S1_Emergency_Close")) {
            s_closet1 = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_closet1_count++;

        /* record */
        mRecordDb->updateRecord("aging", {{"project", "closet1"}}, {{"count", m_closet1_count}});

        s_closet1 = ClosetState::Idle;
        break;
    }
}

void MAgingTestProcess::closet2Process()
{
    switch (s_closet2) {
    case ClosetState::Idle:
        s_closet2 = ClosetState::Open_Closet;
        break;

    case ClosetState::Open_Closet:
    {
        bool ok = dev->track()->cmd_S2_Emergency_Open();
        if (ok) {
            s_closet2 = ClosetState::WaitF_Closet_Open_Done;
        }
    }
        break;
    case ClosetState::WaitF_Closet_Open_Done:
        if (dev->track()->isFuncDone("S2_Emergency_Open")) {
            s_closet2 = ClosetState::Close_Closet;
        }
        break;

    case ClosetState::Close_Closet:
    {
        bool ok = dev->track()->cmd_S2_Emergency_Close();
        if (ok) {
            s_closet2 = ClosetState::WaitF_Closet_Close_Done;
        }
    }
        break;
    case ClosetState::WaitF_Closet_Close_Done:
        if (dev->track()->isFuncDone("S2_Emergency_Close")) {
            s_closet2 = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_closet2_count++;

        /* record */
        mRecordDb->updateRecord("aging", {{"project", "closet2"}}, {{"count", m_closet2_count}});

        s_closet2 = ClosetState::Idle;
        break;
    }
}
