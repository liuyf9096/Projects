#include "m_perfuse_process.h"

MPerfuseProcess::MPerfuseProcess(QObject *parent)
    : MProcessBase{"perfuseProcess", "perfuse", parent}
{
    _init();
}

void MPerfuseProcess::_init()
{
    m_wait = 0;
    s_perfuse = PerfuseState::Idle;
}

void MPerfuseProcess::state_init()
{
    _init();
}

void MPerfuseProcess::onTimer_slot()
{
    switch (s_perfuse) {
    case PerfuseState::Idle:
        s_perfuse = PerfuseState::Perfuse_Water;
        break;

    case PerfuseState::Perfuse_Water:
        dev->stain()->cmd_Perfuse_Water();
        s_perfuse = PerfuseState::WaitF_Perfuse_Water_Done;
        break;
    case PerfuseState::WaitF_Perfuse_Water_Done:
        if (dev->stain()->isFuncDone("Perfuse_Water")) {
            s_perfuse = PerfuseState::Perfuse_Fix;
        }
        break;

    case PerfuseState::Perfuse_Fix:
        dev->stain()->cmd_Perfuse_Fix();
        s_perfuse = PerfuseState::WaitF_Perfuse_Fix_Done;
        break;
    case PerfuseState::WaitF_Perfuse_Fix_Done:
        if (dev->stain()->isFuncDone("Perfuse_Fix")) {
            s_perfuse = PerfuseState::Clean_After_Perfuse_Fix;
        }
        break;

    case PerfuseState::Clean_After_Perfuse_Fix:
        dev->smear()->cmd_WashPoolDrain_Open();
        m_wait = 0;
        s_perfuse = PerfuseState::WaitF_Clean_After_Perfuse_Fix_Down;
        break;
    case PerfuseState::WaitF_Clean_After_Perfuse_Fix_Down:
        if (dev->smear()->isFuncDone("WashPoolDrain_Open")) {
            m_wait++;
            if (m_wait > 20) {
                dev->smear()->cmd_WashPoolDrain_Close();
                s_perfuse = PerfuseState::Perfuse_A1;
            }
        }
        break;

    case PerfuseState::Perfuse_A1:
        dev->stain()->cmd_Perfuse_A1();
        s_perfuse = PerfuseState::WaitF_Perfuse_A1_Done;
        break;
    case PerfuseState::WaitF_Perfuse_A1_Done:
        if (dev->stain()->isFuncDone("Perfuse_A1")) {
            s_perfuse = PerfuseState::Perfuse_B1;
        }
        break;

    case PerfuseState::Perfuse_B1:
        dev->stain()->cmd_Perfuse_B1();
        s_perfuse = PerfuseState::WaitF_Perfuse_B1_Done;
        break;
    case PerfuseState::WaitF_Perfuse_B1_Done:
        if (dev->stain()->isFuncDone("Perfuse_B1")) {
            s_perfuse = PerfuseState::Perfuse_A2;
        }
        break;

    case PerfuseState::Perfuse_A2:
        dev->stain()->cmd_Perfuse_A2();
        s_perfuse = PerfuseState::WaitF_Perfuse_A2_Done;
        break;
    case PerfuseState::WaitF_Perfuse_A2_Done:
        if (dev->stain()->isFuncDone("Perfuse_A2")) {
            s_perfuse = PerfuseState::Perfuse_B2;
        }
        break;

    case PerfuseState::Perfuse_B2:
        dev->stain()->cmd_Perfuse_B2();
        s_perfuse = PerfuseState::WaitF_Perfuse_B2_Done;
        break;
    case PerfuseState::WaitF_Perfuse_B2_Done:
        if (dev->stain()->isFuncDone("Perfuse_B2")) {
            s_perfuse = PerfuseState::Clean_After_Perfuse;
        }
        break;

    case PerfuseState::Clean_After_Perfuse:
//        dev->stain()->cmd_CleanAfterPerfusion();
        s_perfuse = PerfuseState::WaitF_Clean_Done;
        break;
    case PerfuseState::WaitF_Clean_Done:
        if (dev->stain()->isFuncDone("CleanAfterPerfusion")) {
            s_perfuse = PerfuseState::Finish;
        }
        break;

    case PerfuseState::Finish:
        m_isFinished = true;
        mTimer->stop();
        sendOkMessage();
        break;

    case PerfuseState::Error:
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
        break;
    }
}

