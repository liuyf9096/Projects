#include "m_boot_process.h"
#include "module/module_manager.h"
#include "exception/exception_center.h"
#include <QTimer>

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
                s_boot = BootState::Syn_Time;
            }
        }
        break;

    case BootState::Syn_Time:
        dev->track()->cmd_SystemTimeSync();
        s_boot = BootState::WaitF_Syn_Time_Done;
        break;
    case BootState::WaitF_Syn_Time_Done:
        if (dev->track()->isFuncDone("SystemTimeSync")) {
            s_boot = BootState::Check_Exit_Full;
        }
        break;

    case BootState::Check_Exit_Full:
        dev->track()->cmd_CheckSensorValue();
        s_boot = BootState::WaitF_Check_Res_Done;
        break;
    case BootState::WaitF_Check_Res_Done:
        if (dev->track()->isFuncDone("CheckSensorValue") == true) {
            if (dev->track()->checkSensorValue("E2_full") == false) {
                s_boot = BootState::Reset_Track;
                break;
            } else {
                ExceptionCenter::GetInstance()->addException("mian", "Exit2_Full", Exception_Type::UserCode);
            }
        }
        s_boot = BootState::Check_Exit_Full;
        break;

    case BootState::Reset_Track:
        dev->track()->cmd_Reset();
        s_boot = BootState::WaitF_Reset_Done;
        break;
    case BootState::WaitF_Reset_Done:
        if (dev->track()->getFuncResult("Reset") == Func_Done) {
            s_boot = BootState::Check_DMU_Connect_State;
        } else if (dev->track()->getFuncResult("Reset") == Func_Fail) {
            s_boot = BootState::Error;
        }
        break;

    case BootState::Check_DMU_Connect_State:
        if (FMessageCenter::GetInstance()->isDMUConnected() == false) {
            ExceptionCenter::GetInstance()->addException("main", "DMU_Lost_Connection",
                                                         Exception_Type::UserCode);
        }
        s_boot = BootState::Finish;
        break;

    case BootState::Error:
        m_bootTimer->stop();
        m_bootFinished = true;
        qDebug() << "BootState::Error!";
        break;

    case BootState::Finish:
        m_bootTimer->stop();
        m_bootFinished = true;

        DModuleManager::GetInstance()->start();
        s_boot = BootState::Idle;
        break;
    }
}
