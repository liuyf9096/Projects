#include "m_stain_cart.h"
#include "sample/rt_sample_manager.h"
#include "stain/entrance/stain_import.h"
#include "f_common.h"

MStainCart::MStainCart(const QString &mid, QObject *parent)
    : DModuleBase{ mid, "staincart", parent }
{
    m_drytime_sec = 5;
    m_isHeaterEnable = FCommon::GetInstance()->getConfigValue("smear", "dryfan", "enable").toBool();

    state_init();

    mProcessTimer = new QTimer(this);
    mProcessTimer->setInterval(100);
    connect(mProcessTimer, &QTimer::timeout, this, &MStainCart::onProcessTimer_slot);
}

bool MStainCart::isIdle()
{
    if (s_state == Process::Idle && m_slide.isNull()) {
        return true;
    }
    return false;
}

bool MStainCart::isLoaded()
{
    if (m_slide.isNull()) {
        return false;
    } else {
        return true;
    }
}

bool MStainCart::receiveNewSlide(QSharedPointer<RtSlide> slide)
{
    if (m_slide.isNull()) {
        m_slide = slide;
        recordModuleSlideState("stain_slide", "doing_slide", slide);
        return true;
    } else {
        qWarning() << "Stain cart is buzy.";
    }
    return false;
}

void MStainCart::setHeaterParams(const QJsonObject &obj)
{
    if (obj.contains("time")) {
        int time = obj.value("time").toInt();
        m_drytime_sec = time;
        qDebug() << "Set Smear Dry time:" << m_drytime_sec << "s";
    }
    if (obj.contains("temp")) {
        int temp = obj.value("temp").toInt();
        dev->sample()->cmd_SetHeaterTempValue(0, temp);
        qDebug() << "Set Smear Dry temp:" << temp << "`C";
    }
}

void MStainCart::start()
{
    mProcessTimer->start();
}

void MStainCart::reset()
{
    state_init();
}

void MStainCart::stop()
{
    mProcessTimer->stop();
}

void MStainCart::state_init()
{
    s_state = Process::Idle;
}

void MStainCart::onProcessTimer_slot()
{
    switch (s_state) {
    case Process::Idle:
        if (m_slide.isNull() == false) {
            logProcess("StainCart", 1, "Start", m_slide->slide_id());
            s_state = Process::Send_Move_To_Stain_Cmd;
        }
        break;

        /* 1. Move To Stain Import */
    case Process::Send_Move_To_Stain_Cmd:
        if (m_slide->isPrintOnly()) {
            dev->sample()->cmd_StainCart_ToStainImportWithoutHeat();
            m_api = "StainCart_ToStainImportWithoutHeat";
        } else {
            if (m_isHeaterEnable) {
                dev->sample()->cmd_Heater_Open();
            }
            dev->sample()->cmd_StainCart_ToStainImport(m_drytime_sec * 1000);
            m_api = "StainCart_ToStainImport";
        }
        s_state = Process::WaitF_Move_To_Stain_Done;
        break;
    case Process::WaitF_Move_To_Stain_Done:
        if (dev->sample()->isFuncDone(m_api)) {
            if (m_isHeaterEnable) {
                dev->sample()->cmd_Heater_Close();
            }

            bool ok = mImport->addNewSlide(m_slide);
            if (ok) {
                logProcess("StainCart", 2, QString("Arrive at Import, wait for Unload. %1").arg(m_slide->slide_id()));
                s_state = Process::WaitF_Unload_Slide_Done;
            }
        }
        break;

        /* 3. Wait For Unload */
    case Process::WaitF_Unload_Slide_Done:
        if (mImport->isEmpty()) {
            s_state = Process::Send_Reset_Cmd;
        }
        break;

        /* 4. Reset */
    case Process::Send_Reset_Cmd:
        logProcess("StainCart", 3, "Cart Reset");
        dev->sample()->cmd_StainCart_Reset();
        s_state = Process::WaitF_Reset_Done;
        break;
    case Process::WaitF_Reset_Done:
        if (dev->sample()->isFuncDone("StainCart_Reset")) {
            s_state = Process::Finish;
        }
        break;

    case Process::Finish:
        m_slide = nullptr;
        logProcess("StainCart", 99, "Finish");
        s_state = Process::Idle;
        break;
    }
}
