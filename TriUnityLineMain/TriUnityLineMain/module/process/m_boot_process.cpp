#include "m_boot_process.h"
#include "module/module_manager.h"

MBootProcess::MBootProcess(QObject *parent)
    : DModuleBase("boot", "boot", parent)
{
    state_init();

    m_bootTimer = new QTimer(this);
    m_bootTimer->setInterval(100);
    connect(m_bootTimer, &QTimer::timeout,
            this, &MBootProcess::onBootTimer_slot);
}

void MBootProcess::state_init()
{
    m_bootFinished = false;
    s_boot = BootState::Idle;
}

void MBootProcess::start()
{
    m_bootTimer->start();
    DModuleBase::start();
}

void MBootProcess::reset()
{
    state_init();
    DModuleBase::reset();
}

void MBootProcess::stop()
{
    m_bootTimer->stop();
    DModuleBase::stop();
}

void MBootProcess::onBootTimer_slot()
{
    switch (s_boot) {
    case BootState::Idle:
        if (m_bootFinished == false) {
            if (FMessageCenter::GetInstance()->isCanbusConnected()) {
                s_boot = BootState::Syn_Time_1;
            }
        }
        break;

    case BootState::Syn_Time_1:
        if (dev->track1()->isEnable()) {
            dev->track1()->cmd_SystemTimeSync();
            s_boot = BootState::WaitF_Syn_Time_1_Done;
        } else {
            s_boot = BootState::Syn_Time_2;
        }
        break;
    case BootState::WaitF_Syn_Time_1_Done:
        if (dev->track1()->isFuncDone("SystemTimeSync")) {
            s_boot = BootState::Syn_Time_2;
        }
        break;

    case BootState::Syn_Time_2:
        if (dev->track2()->isEnable()) {
            dev->track2()->cmd_SystemTimeSync();
            s_boot = BootState::WaitF_Syn_Time_2_Done;
        } else {
            s_boot = BootState::Reset_Track_1;
        }
        break;
    case BootState::WaitF_Syn_Time_2_Done:
        s_boot = BootState::Reset_Track_1;
        break;

    case BootState::Reset_Track_1:
        if (dev->track1()->isEnable()) {
            dev->track1()->cmd_Reset();
            s_boot = BootState::WaitF_Reset1_Done;
        } else {
            s_boot = BootState::Reset_Track_2;
        }
        break;
    case BootState::WaitF_Reset1_Done:
        if (dev->track1()->isFuncDone("Reset")) {
            s_boot = BootState::Reset_Track_2;
        }
        break;

    case BootState::Reset_Track_2:
        if (dev->track2()->isEnable()) {
            dev->track2()->cmd_Reset();
            s_boot = BootState::WaitF_Reset2_Done;
        } else {
            s_boot = BootState::Finish;
        }
        break;
    case BootState::WaitF_Reset2_Done:
        if (dev->track2()->isFuncDone("Reset")) {
            s_boot = BootState::Finish;
        }
        break;

    case BootState::Finish:
        m_bootTimer->stop();
        m_bootFinished = true;

        DModuleManager::GetInstance()->start();
        s_boot = BootState::Idle;
        break;
    }
}
