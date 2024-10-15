#include "m_import.h"
#include "cart/carts_manager.h"
#include "exception/exception_center.h"
#include "f_common.h"

MImport::MImport(QObject *parent)
    : DModuleBase("import", "import", parent)
{
    m_isReceiveNewEnable = true;
    mMaxFailCount = FCommon::GetInstance()->getConfigValue("import", "max_fail_count").toInt(3);
    if (mMaxFailCount < 1) {
        mMaxFailCount = 3;
    }
    qDebug() << "Import max fail count:" << mMaxFailCount;

    state_init();

    m_importTimer = new QTimer(this);
    m_importTimer->setInterval(100);
    connect(m_importTimer, &QTimer::timeout,
            this, &MImport::onImportTimer_slot);

    auto track = RtDeviceManager::GetInstance()->track();
    connect(track, &DTrack::sonserInfo_signal, this, [=](){
        m_sensorUpdate = true;
    });
    connect(track, &DTrack::onFunctionFinished_signal,
            this, &MImport::onFunctionFinished_slot);
}

void MImport::state_init()
{
    mCart = nullptr;
    s_imState = ImState::Idle;
    m_sensorUpdate = false;
    m_failCount = 0;
    m_checkSensorCount = 0;
}

void MImport::startReceieNewRack()
{
    m_isReceiveNewEnable = true;
}

void MImport::stopReceiveNewRack()
{
    m_isReceiveNewEnable = false;
    dev->track()->setCheckSensorEnable(mModuleId, false);
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
    if (dev->track()->isResetOk() == false) {
        return;
    }

    if (FMessageCenter::GetInstance()->isDMUConnected() == false) {
        return;
    }

    switch (s_imState) {
    case ImState::Idle:
        mCart = CartsManager::GetInstance()->getSpareCart();
        if (mCart != nullptr) {
            m_failCount = 0;
            logProcess("Import", 1, "Get Spare cart", mCart->mid());
            s_imState = ImState::Check_New_Sample_Rack;
        }
        break;

    case ImState::Check_New_Sample_Rack:
        if (m_isReceiveNewEnable == true) {
            m_checkSensorCount = 0;
            m_sensorUpdate = false;
            dev->track()->setCheckSensorEnable(mModuleId, true);

            s_imState = ImState::WaitF_New_Sample_Rack;
        }
        break;

    case ImState::WaitF_New_Sample_Rack:
        if (m_isReceiveNewEnable == false) {
            dev->track()->setCheckSensorEnable(mModuleId, false);
            s_imState = ImState::Check_New_Sample_Rack;
        } else if (m_sensorUpdate == true) {
            m_sensorUpdate = false;

            if (dev->track()->checkSensorValue("Import_new_rack") == true) {
                m_checkSensorCount++;
                if (m_checkSensorCount > 1) {
                    dev->track()->setCheckSensorEnable(mModuleId, false);
                    logProcess("Import", 2, "Get New Sample");
                    s_imState = ImState::Send_Cart_GoTo_Import_Pos;
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
            s_imState = ImState::WaitF_Cart_GoTo_Import_Pos_Ready;
        }
        break;
    }
    case ImState::WaitF_Cart_GoTo_Import_Pos_Ready:
        if (mCart->isCmd_PosImport_done()) {
            m_failCount = 0;
            CartsManager::GetInstance()->updateCartRelativePos();
            s_imState = ImState::Check_New_Sample_Rack_1;
        }
        break;

    case ImState::Check_New_Sample_Rack_1:
        m_sensorUpdate = false;
        dev->track()->setCheckSensorEnable(mid(), true);

        s_imState = ImState::WaitF_New_Sample_Rack_1;
        break;
    case ImState::WaitF_New_Sample_Rack_1:
        if (m_sensorUpdate == true) {
            m_sensorUpdate = false;

            if (dev->track()->checkSensorValue("Import_new_rack") == true) {
                dev->track()->setCheckSensorEnable(mid(), false);
                logProcess("Import", 3, "Get New Sample_1");
                s_imState = ImState::Send_Load_Rack_Cmd;
            }
        }
        break;

    case ImState::Send_Load_Rack_Cmd:
        logProcess("Import", 4, "Load Sample");
        dev->track()->cmd_Import_Load();
        s_imState = ImState::WaitF_Load_Rack_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Cmd_Done:
        if (dev->track()->getFuncResult("Import_Load") == Func_Done) {
            m_failCount = 0;
            logProcess("Import", 5, "Load Finish");
            s_imState = ImState::New_Rack;
        } else if (dev->track()->getFuncResult("Import_Load") == Func_Fail) {
            m_failCount++;
            logProcess("Import", 5, "Load Fail and Reset", QString("Fail count:%1").arg(m_failCount));
            s_imState = ImState::Send_Load_Rack_Reset_Cmd;
        }
        break;

    case ImState::New_Rack:
        mCart->newRack();
        s_imState = ImState::Send_Load_Rack_Reset_Cmd;
        break;

    case ImState::Send_Load_Rack_Reset_Cmd:
        logProcess("Import", 6, "Load Reset");
        dev->track()->cmd_Import_Reset();
        s_imState = ImState::WaitF_Load_Rack_Reset_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Reset_Cmd_Done:
        if (dev->track()->isFuncDone("Import_Reset")) {
            if (mCart->rack() == nullptr) {
                if (m_failCount >= mMaxFailCount) {
                    logProcess("Import", 8, "Error");

                    ExceptionCenter::GetInstance()->addException(dev->track()->deviceID(),
                                                                 "Load_Rack",
                                                                 Exception_Type::UserCode, "101");
                    s_imState = ImState::Error;
                } else {
                    logProcess("Import", 7, "Try Again");
                    s_imState = ImState::Check_New_Sample_Rack;
                }
            } else {
                logProcess("Import", 99, "Reset Finish");
                s_imState = ImState::Idle;
            }
        }
        break;

    case ImState::Error:
        if (m_failCount == 0) {
            s_imState = ImState::Send_Load_Rack_Cmd;
        }
        break;
    }
}

void MImport::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    Q_UNUSED(resValue)

    if (api == "Import_Load") {
        ExceptionCenter::GetInstance()->sendRemoveExceptionMessage("210130200");
    }
}
