#include "m_import.h"
#include "cart/carts_manager.h"
#include "rack/rt_rack.h"

MImport::MImport(const QString &mid, QObject *parent)
    : DModuleBase(mid, "Import", parent)
{
    m_isReceiveNewEnable = false;

    state_init();
    m_WaitTime = 0;

    m_importTimer = new QTimer(this);
    m_importTimer->setInterval(100);
    connect(m_importTimer, &QTimer::timeout,
            this, &MImport::onImportTimer_slot);

    auto track = RtDeviceManager::GetInstance()->track();
    connect(track, &DTrack::sonserInfo_signal, this, [=](){
        m_sensorUpdate = true;
    });
}

void MImport::state_init()
{
    mCart = nullptr;
    s_imState = ImState::Idle;
}

void MImport::startReceieNewRack()
{
    m_isReceiveNewEnable = true;
    dev->track()->setCheckSensorEnable(mModuleId, true);
    qInfo() << "Start Detect New Rack.";
}

void MImport::stopReceiveNewRack()
{
    m_isReceiveNewEnable = false;
    dev->track()->setCheckSensorEnable(mModuleId, false);

    qInfo() << "Stop Detect New Rack.";
}

void MImport::cancel()
{
    if (mCart) {
        mCart->abort();
    }
}

void MImport::setWaitTime(int interval)
{
    if (interval >= 0 && interval < 50) {
        m_WaitTime = interval;
        qInfo() << "set Import waitTime:" << interval << "s";
    } else {
        qInfo() << "set Import waitTime error." << interval;
    }
}

bool MImport::clearImportError()
{
    if (s_imState == ImState::Error) {
        m_failCount = 0;
        return true;
    }
    return false;
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

    switch (s_imState) {
    case ImState::Idle:
    {
        bool ok;
        auto cart = CartsManager::GetInstance()->getSpareCart(&ok);
        if (ok && cart) {
            mCart = cart;
            m_failCount = 0;
            logProcess("Import", 1, "Get Spare cart", mCart->mid());
            s_imState = ImState::Check_New_Sample_Rack;
        }
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
                if (m_checkSensorCount > m_WaitTime * 5) { // 200*5=1000s
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
            dev->sample()->cmd_AirCompressor_Open();//todo
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
        if (m_isReceiveNewEnable == true) {
            m_sensorUpdate = false;
            dev->track()->setCheckSensorEnable(mid(), true);

            s_imState = ImState::WaitF_New_Sample_Rack_1;
        }
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
        logProcess("Import", 3, "Load Sample");
        dev->track()->cmd_Import_Load();
        s_imState = ImState::WaitF_Load_Rack_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Cmd_Done:
        if (dev->track()->getFuncResult("Import_Load") == Func_Done) {
            m_failCount = 0;
            auto rack = mCart->newRack();
            if (rack) {
                JPacket p(PacketType::Notification);
                p.module = "Rack";
                p.api = "RackInfo";

                QJsonObject obj;
                obj.insert("rack_id", rack->rackid());
                obj.insert("barcode", rack->barcode());
                p.paramsValue = obj;
                FMessageCenter::GetInstance()->sendUIMessage(p);

                if (m_isReceiveNewEnable == false) {
                    rack->abort();
                }
            }

            logProcess("Import", 4, "Load Finish");
            ExceptionCenter::GetInstance()->removeException("LoadNewRack_101", true);
            s_imState = ImState::Send_Load_Rack_Reset_Cmd;
        } else if (dev->track()->getFuncResult("Import_Load") == Func_Fail) {
            m_failCount++;
            if (m_failCount > 2) {
                logProcess("Import", -1, "Load Fail and stop");

                Exception e("LoadNewRack_101", E_Level::Alarm, 101);
                e.e_msg = "can NOT push New Rack correctly.";
                ExceptionCenter::GetInstance()->sendExceptionMessage(e);

                s_imState = ImState::Error;
            } else {
                logProcess("Import", 4, "Load Fail and try again", QString("Fail count:%1").arg(m_failCount));
                s_imState = ImState::Handle_Load_Fail;
            }
        }
        break;

    case ImState::Send_Load_Rack_Reset_Cmd:
        logProcess("Import", 5, "Load Reset");
        dev->track()->cmd_Import_Reset();
        s_imState = ImState::WaitF_Load_Rack_Reset_Cmd_Done;
        break;
    case ImState::WaitF_Load_Rack_Reset_Cmd_Done:
        if (dev->track()->isFuncDone("Import_Reset")) {
            s_imState = ImState::Finish;
        }
        break;

    case ImState::Handle_Load_Fail:
        dev->track()->cmd_Import_Reset();
        s_imState = ImState::WaitF_Hanlde_Load_Fail_Done;
        break;
    case ImState::WaitF_Hanlde_Load_Fail_Done:
        if (dev->track()->isFuncDone("Import_Reset")) {
            s_imState = ImState::Check_New_Sample_Rack_1;
        }
        break;

    case ImState::Finish:
        logProcess("Import", 99, "Finish");
        mCart = nullptr;
        s_imState = ImState::Idle;
        break;

    case ImState::Error:
        if (m_failCount == 0) {
            s_imState = ImState::Send_Load_Rack_Cmd;
        }
        break;
    }
}
