#include "m_aging_test_process.h"
#include "settings/f_settings.h"
#include "cart/carts_manager.h"

MAgingTestProcess::MAgingTestProcess(QObject *parent)
    : DModuleBase("agingtest", "agingtest", parent)
{
    state_init();

    settings = FSettings::GetInstance();
    m_count = settings->agingCount();
    m_closet1_count = settings->closet1Count();
    m_closet2_count = settings->closet2Count();

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
        dev->track1()->cmd_CheckSensorValue();
        s_aging = TrackState::WaitF_Check_Done;
        break;
    case TrackState::WaitF_Check_Done:
        if (dev->track1()->isFuncDone("CheckSensorValue")) {
            if (dev->track1()->checkSensorValue("Import_new_rack") == true) {
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
        dev->track1()->cmd_Import_Load();
        s_aging = TrackState::WaitF_Load_Rack_Done;
        break;
    case TrackState::WaitF_Load_Rack_Done:
        if (dev->track1()->isFuncDone("Import_Load")) {
            dev->track1()->cmd_Import_Reset();
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
        dev->track1()->cmd_S1_Scan_Open(500);
        s_aging = TrackState::WaitF_Open_Scan1_Done;
        break;
    case TrackState::WaitF_Open_Scan1_Done:
        if (dev->track1()->isFuncDone("S1_Scan_Open")) {
            s_aging = TrackState::Move_To_Test1_Pos;
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
        if (FSettings::GetInstance()->getStationCount() > 1) {
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
        dev->track2()->cmd_S2_Scan_Open(500);
        s_aging = TrackState::WaitF_Open_Scan2_Done;
        break;
    case TrackState::WaitF_Open_Scan2_Done:
        if (dev->track2()->isFuncDone("S2_Scan_Open")) {
            s_aging = TrackState::Move_To_Test2_Pos;
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
        if (FSettings::GetInstance()->getStationCount() > 2) {
            mCart->req_PosSxScan("S3", m_index);
            s_aging = TrackState::WaitF_Scan3_Pos_Done;
        } else {
            s_aging = TrackState::Move_To_Export_Pos;
        }
        break;
    case TrackState::WaitF_Scan3_Pos_Done:
        if (mCart->isCmd_PosScan_done("S3")) {
            s_aging = TrackState::Open_Scan3;
        }
        break;

    case TrackState::Open_Scan3:
        dev->track2()->cmd_S3_Scan_Open(500);
        s_aging = TrackState::WaitF_Open_Scan3_Done;
        break;
    case TrackState::WaitF_Open_Scan3_Done:
        if (dev->track2()->isFuncDone("S3_Scan_Open")) {
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
        if (FSettings::GetInstance()->getExitCount() == 1) {
            m_exitIndex = 1;
            mCart->req_PosExitEx("E1");
        } else {
            if (m_count % 2 == 0) {
                m_exitIndex = 1;
                mCart->req_PosExitEx("E1");
            } else {
                m_exitIndex = 2;
                mCart->req_PosExitEx("E2");
            }
        }
        s_aging = TrackState::WaitF_Export_Done;
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
            dev->track1()->cmd_Exit_E1_Unload();
        } else if (m_exitIndex == 2) {
            dev->track2()->cmd_Exit_E2_Unload();
        }
        s_aging = TrackState::WaitF_UnLoad_Rack_Done;
        break;
    case TrackState::WaitF_UnLoad_Rack_Done:
        if (m_exitIndex == 1 && dev->track1()->isFuncDone("Exit_E1_Unload")) {
            s_aging = TrackState::Reset_Cart;
        } else if (m_exitIndex == 2 && dev->track2()->isFuncDone("Exit_E2_Unload")) {
            s_aging = TrackState::Reset_Cart;
        }
        break;

    case TrackState::Reset_Cart:
        m_count++;
        settings->saveAgingCount(m_count);
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
        bool ok = dev->track1()->cmd_Reset();
        if (ok) {
            s_aging = TrackState::WaitF_Carts_Done;
        }
        break;
    }
    case TrackState::WaitF_Carts_Done:
        if (dev->track1()->isFuncDone("Reset")) {
            s_aging = TrackState::Idle;
        }
        break;

    case TrackState::Finish:
        m_index++;
        if (m_index >= 10) {
            m_index = 0;
        }
        s_aging = TrackState::Idle;
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
        dev->track1()->cmd_S1_Emergency_Open();
        s_closet1 = ClosetState::WaitF_Closet_Open_Done;
        break;
    case ClosetState::WaitF_Closet_Open_Done:
        if (dev->track1()->isFuncDone("S1_Emergency_Open")) {
            s_closet1 = ClosetState::Close_Closet;
        }
        break;

    case ClosetState::Close_Closet:
        dev->track1()->cmd_S1_Emergency_Close();
        s_closet1 = ClosetState::WaitF_Closet_Close_Done;
        break;
    case ClosetState::WaitF_Closet_Close_Done:
        if (dev->track1()->isFuncDone("S1_Emergency_Close")) {
            s_closet1 = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_closet1_count++;
        settings->saveCloset1Count(m_closet1_count);
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
        dev->track2()->cmd_S2_Emergency_Open();
        s_closet2 = ClosetState::WaitF_Closet_Open_Done;
        break;
    case ClosetState::WaitF_Closet_Open_Done:
        if (dev->track2()->isFuncDone("S2_Emergency_Open")) {
            s_closet2 = ClosetState::Close_Closet;
        }
        break;

    case ClosetState::Close_Closet:
        dev->track2()->cmd_S2_Emergency_Close();
        s_closet2 = ClosetState::WaitF_Closet_Close_Done;
        break;
    case ClosetState::WaitF_Closet_Close_Done:
        if (dev->track2()->isFuncDone("S2_Emergency_Close")) {
            s_closet2 = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_closet2_count++;
        settings->saveCloset2Count(m_closet2_count);
        s_closet2 = ClosetState::Idle;
        break;
    }
}
