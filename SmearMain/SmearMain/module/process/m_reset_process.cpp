#include "m_reset_process.h"

MResetProcess::MResetProcess(QObject *parent)
    : MProcessBase{"resetProcess", "reset", parent}
{
    _init();
}

void MResetProcess::_init()
{
    s_rstTrask = ResetState::Idle;
    s_rstSampling = ResetState::Idle;
    s_rstSmear = ResetState::Idle;
    s_rstStain = ResetState::Idle;
    s_rstPrint = ResetState::Idle;

    m_ProcessMap.clear();
    m_ProcessMap.insert("resetTrack", false);
    m_ProcessMap.insert("resetSampling", false);
    m_ProcessMap.insert("resetSmear", false);
    m_ProcessMap.insert("resetStain", false);
//    m_ProcessMap.insert("resetPrint", false);
}

void MResetProcess::state_init()
{
    _init();
}

void MResetProcess::onTimer_slot()
{
    resetTrackProcess();
    resetSamplingProcess();
    resetSmearProcess();
    resetStainProcess();
//    resetPrintProcess();

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

void MResetProcess::resetTrackProcess()
{
    switch (s_rstTrask) {
    case ResetState::Idle:
        s_rstTrask = ResetState::Reset_Device_Machine;
        break;
    case ResetState::Reset_Device_Machine:
        dev->track()->cmd_Reset();
        s_rstTrask = ResetState::WaitF_Reset_Device_Done;
        break;
    case ResetState::WaitF_Reset_Device_Done:
        if (dev->track()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("resetTrack", true);
            s_rstTrask = ResetState::Finish;
        } else if (dev->track()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_rstTrask = ResetState::Error;
        }
        break;
    case ResetState::Finish:
        break;
    case ResetState::Error:
        break;
    }
}

void MResetProcess::resetSamplingProcess()
{
    switch (s_rstSampling) {
    case ResetState::Idle:
        s_rstSampling = ResetState::Reset_Device_Machine;
        break;
    case ResetState::Reset_Device_Machine:
        dev->sample()->cmd_Reset();
        s_rstSampling = ResetState::WaitF_Reset_Device_Done;
        break;
    case ResetState::WaitF_Reset_Device_Done:
        if (dev->sample()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("resetSampling", true);
            s_rstSampling = ResetState::Finish;
        } else if (dev->sample()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_rstSampling = ResetState::Error;
        }
        break;
    case ResetState::Finish:
        break;
    case ResetState::Error:
        break;
    }
}

void MResetProcess::resetSmearProcess()
{
    switch (s_rstSmear) {
    case ResetState::Idle:
        s_rstSmear = ResetState::Reset_Device_Machine;
        break;
    case ResetState::Reset_Device_Machine:
        dev->smear()->cmd_Reset();
        s_rstSmear = ResetState::WaitF_Reset_Device_Done;
        break;
    case ResetState::WaitF_Reset_Device_Done:
        if (dev->smear()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("resetSmear", true);
            s_rstSmear = ResetState::Finish;
        } else if (dev->smear()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_rstSmear = ResetState::Error;
        }
        break;
    case ResetState::Finish:
        break;
    case ResetState::Error:
        break;
    }
}

void MResetProcess::resetStainProcess()
{
    switch (s_rstStain) {
    case ResetState::Idle:
        s_rstStain = ResetState::Reset_Device_Machine;
        break;
    case ResetState::Reset_Device_Machine:
        dev->stain()->cmd_Reset();
        s_rstStain = ResetState::WaitF_Reset_Device_Done;
        break;
    case ResetState::WaitF_Reset_Device_Done:
        if (dev->stain()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("resetStain", true);
            s_rstStain = ResetState::Finish;
        } else if (dev->stain()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_rstStain = ResetState::Error;
        }
        break;
    case ResetState::Finish:
        break;
    case ResetState::Error:
        break;
    }
}

void MResetProcess::resetPrintProcess()
{
    switch (s_rstPrint) {
    case ResetState::Idle:
        s_rstPrint = ResetState::Reset_Device_Machine;
        break;
    case ResetState::Reset_Device_Machine:
        dev->print()->cmd_Reset();
        s_rstPrint = ResetState::WaitF_Reset_Device_Done;
        break;
    case ResetState::WaitF_Reset_Device_Done:
        if (dev->print()->getFuncResult("PrinterReset") == Func_Done) {
            m_ProcessMap.insert("resetPrint", true);
            s_rstPrint = ResetState::Finish;
        } else if (dev->print()->getFuncResult("PrinterReset") == Func_Fail) {
            m_isError = true;
            s_rstPrint = ResetState::Error;
        }
        break;
    case ResetState::Finish:
        break;
    case ResetState::Error:
        break;
    }
}

