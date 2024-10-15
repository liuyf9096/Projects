#include "m_import.h"
#include "cart/carts_manager.h"

MImport::MImport(QObject *parent)
    : DModuleBase("import", "import", parent)
{
    m_isReceiveNewEnable = true;

    state_init();

    m_importTimer = new QTimer(this);
    m_importTimer->setInterval(100);
    connect(m_importTimer, &QTimer::timeout,
            this, &MImport::onImportTimer_slot);

    auto track1 = RtDeviceManager::GetInstance()->track1();
    connect(track1, &DTrack1::sonserInfo_signal, this, [=](){
        m_sensorUpdate = true;
    });
}

void MImport::startReceieNewRack()
{
    m_isReceiveNewEnable = true;
}

void MImport::stopReceiveNewRack()
{
    m_isReceiveNewEnable = false;
    dev->track1()->setCheckSensorEnable(mModuleId, false);
}

void MImport::state_init()
{
    mCart = nullptr;
    m_imState = ImState::Idle;
}

void MImport::start()
{
    m_importTimer->start();
    DModuleBase::start();
}

void MImport::reset()
{
    state_init();
    DModuleBase::reset();
}

void MImport::stop()
{
    m_importTimer->stop();
    DModuleBase::stop();
}

void MImport::onImportTimer_slot()
{
    if (dev->track1()->isResetOk() == false) {
        return;
    }

    switch (m_imState) {
    case ImState::Idle:
        mCart = CartsManager::GetInstance()->getSpareCart();
        if (mCart != nullptr) {
            m_importFailCount = 0;
            logProcess("Import", 1, "Get Spare cart", mCart->mid());
            m_imState = ImState::Check_New_Sample_Rack;
        }
        break;

    case ImState::Check_New_Sample_Rack:
        if (m_isReceiveNewEnable == true) {
            m_checkSensorCount = 0;
            m_sensorUpdate = false;
            dev->track1()->setCheckSensorEnable(mModuleId, true);

            m_imState = ImState::WaitF_New_Sample_Rack;
        }
        break;

    case ImState::WaitF_New_Sample_Rack:
        if (m_isReceiveNewEnable == false) {
            dev->track1()->setCheckSensorEnable(mModuleId, false);
            m_imState = ImState::Check_New_Sample_Rack;
        } else if (m_sensorUpdate == true) {
            m_sensorUpdate = false;

            if (dev->track1()->checkSensorValue("Import_new_rack") == true) {
                m_checkSensorCount++;
                if (m_checkSensorCount > 1) {
                    dev->track1()->setCheckSensorEnable(mModuleId, false);
                    logProcess("Import", 2, "Get New Sample");
                    m_imState = ImState::Send_Cart_GoTo_Import_Pos;
                }
            } else {
                if (m_checkSensorCount > 0) {
                    m_checkSensorCount--;
                }
            }
        }
        break;

    case ImState::Send_Cart_GoTo_Import_Pos:
    {
        bool ok = mCart->req_PosImport();
        if (ok) {
            m_imState = ImState::WaitF_Cart_GoTo_Import_Pos_Ready;
        }
        break;
    }
    case ImState::WaitF_Cart_GoTo_Import_Pos_Ready:
        if (mCart->isCmd_PosImport_done()) {
            m_imState = ImState::Check_New_Sample_Rack_1;
        }
        break;

    case ImState::Check_New_Sample_Rack_1:
        m_sensorUpdate = false;
        dev->track1()->setCheckSensorEnable(mid(), true);

        m_imState = ImState::WaitF_New_Sample_Rack_1;
        break;
    case ImState::WaitF_New_Sample_Rack_1:
        if (m_sensorUpdate == true) {
            m_sensorUpdate = false;

            if (dev->track1()->checkSensorValue("Import_new_rack") == true) {
                dev->track1()->setCheckSensorEnable(mid(), false);
                logProcess("Import", 3, "Get New Sample_1");
                m_imState = ImState::Send_Load_Rack_Cmd;

            }
        }
        break;

    case ImState::Send_Load_Rack_Cmd:
        logProcess("Import", 4, "Load Sample");
        dev->track1()->cmd_Import_Load();
        m_imState = ImState::WaitF_Load_Rack_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Cmd_Done:
        if (dev->track1()->getFuncResult("Import_Load") == Func_Done) {
            m_importFailCount = 0;
            logProcess("Import", 5, "Load Finish");
            m_imState = ImState::New_Rack;
        } else if (dev->track1()->getFuncResult("Import_Load") == Func_Fail) {
            m_importFailCount++;
            logProcess("Import", 5, "Load Fail and Reset", QString("Fail count:%1").arg(m_importFailCount));
            m_imState = ImState::Send_Load_Rack_Reset_Cmd;
        }
        break;

    case ImState::New_Rack:
        mCart->newRack();
        m_imState = ImState::Send_Load_Rack_Reset_Cmd;
        break;

    case ImState::Send_Load_Rack_Reset_Cmd:
        logProcess("Import", 6, "Load Reset");
        dev->track1()->cmd_Import_Reset();
        m_imState = ImState::WaitF_Load_Rack_Reset_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Reset_Cmd_Done:
        if (dev->track1()->isFuncDone("Import_Reset")) {
            if (mCart->rack() == nullptr) {
                if (m_importFailCount > 2) {
                    logProcess("Import", 8, "Error");
                    m_imState = ImState::Error;
                } else {
                    logProcess("Import", 7, "Try Again");
                    m_imState = ImState::Check_New_Sample_Rack;
                }
            } else {
                logProcess("Import", 99, "Reset Finish");
                m_imState = ImState::Idle;
            }
        }
        break;

    case ImState::Error:
        break;
    }
}
