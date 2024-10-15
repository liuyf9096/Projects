#include "m_clean_liquid_system_process.h"

MCleanLiquidSystemProcess::MCleanLiquidSystemProcess(QObject *parent)
    : MProcessBase{"cleanLiquidSystemProcess", "cleanLiquid", parent}
{
    _init();
}

void MCleanLiquidSystemProcess::_init()
{
    s_Sampling = Idle;
    s_Smear = Idle;
    s_Stain = Idle;

    m_ProcessMap.clear();
    m_ProcessMap.insert("sampling", false);
    m_ProcessMap.insert("smear", false);
    m_ProcessMap.insert("stain", false);
}

void MCleanLiquidSystemProcess::state_init()
{
    _init();
}

void MCleanLiquidSystemProcess::onTimer_slot()
{
    cleanSmearProcess();
    cleanStainProcess();
    cleanSamplingProcess();

    if (checkStateAllFinished() == true) {
        m_isFinished = true;
        mTimer->stop();
        sendOkMessage();
    } else if (m_isError == true) {
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
    }
}

void MCleanLiquidSystemProcess::cleanSamplingProcess()
{
    switch (s_Sampling) {
    case Idle:
        s_Sampling = Prepare;
        break;

    case Prepare:
        dev->smear()->cmd_WashPoolDrain_Open();
        s_Sampling = Prepare_Done;
        break;
    case Prepare_Done:
        if (dev->smear()->isFuncDone("WashPoolDrain_Open")) {
            s_Sampling = Clean_Liquid_System;
        }
        break;

    case Clean_Liquid_System:
        dev->sample()->cmd_CleanLiquidSystem();
        s_Sampling = WaitF_Clean_System_Done;
        break;
    case WaitF_Clean_System_Done:
        if (dev->sample()->getFuncResult("CleanLiquidSystem") == Func_Done) {
            m_ProcessMap.insert("sampling", true);
            s_Sampling = Closing;
        } else if (dev->sample()->getFuncResult("CleanLiquidSystem") == Func_Fail) {
            m_isError = false;
            s_Sampling = Error;
        }
        break;

    case Closing:
        dev->smear()->cmd_WashPoolDrain_Close();
        s_Sampling = Closing_Done;
        break;
    case Closing_Done:
        if (dev->smear()->isFuncDone("WashPoolDrain_Close")) {
            s_Sampling = Finish;
        }
        break;

    case Finish:
        break;
    case Error:
        break;
    }
}

void MCleanLiquidSystemProcess::cleanSmearProcess()
{
    switch (s_Smear) {
    case Idle:
        if (s_Sampling == Finish) {
            s_Smear = Clean_Liquid_System;
        }
        break;

    case Clean_Liquid_System:
        dev->smear()->cmd_CleanLiquidSystem();
        s_Smear = WaitF_Clean_System_Done;
        break;
    case WaitF_Clean_System_Done:
        if (dev->smear()->getFuncResult("CleanLiquidSystem") == Func_Done) {
            m_ProcessMap.insert("smear", true);
            s_Smear = Finish;
        } else if (dev->smear()->getFuncResult("CleanLiquidSystem") == Func_Fail) {
            m_isError = false;
            s_Smear = Error;
        }
        break;

    case Finish:
        break;
    case Error:
        break;
    default:
        break;
    }
}

void MCleanLiquidSystemProcess::cleanStainProcess()
{
    switch (s_Stain) {
    case Idle:
        s_Stain = Clean_Liquid_System;
        break;

    case Clean_Liquid_System:
        dev->stain()->cmd_Perfuse_LiquidSystem();
        s_Stain = WaitF_Clean_System_Done;
        break;
    case WaitF_Clean_System_Done:
        if (dev->stain()->getFuncResult("CleanLiquidSystem") == Func_Done) {
            m_ProcessMap.insert("stain", true);
            s_Stain = Finish;
        } else if (dev->stain()->getFuncResult("CleanLiquidSystem") == Func_Fail) {
            m_isError = false;
            s_Stain = Error;
        }
        break;

    case Finish:
        break;
    case Error:
        break;
    default:
        break;
    }
}
