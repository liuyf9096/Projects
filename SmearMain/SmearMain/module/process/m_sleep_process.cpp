#include "m_sleep_process.h"
#include "stain/recyclebox/m_recyclebox_mgr.h"
#include "module_manager.h"
#include "stain/slot/slots_manager.h"
#include "light/m_lights.h"
#include "track/track_manager.h"
#include "f_common.h"

MSleepProcess::MSleepProcess(QObject *parent)
    : MProcessBase{"sleepprocess", "sleep", parent}
{
    m_isUnited = FCommon::GetInstance()->isUnited();
    m_isTrackEn = FCommon::GetInstance()->isTrackEnable();
    m_isPrintEn = FCommon::GetInstance()->isPrintEnable();

    _init();
}

void MSleepProcess::state_init()
{
    _init();
}

void MSleepProcess::_init()
{
    m_progress = 0;

    s_bTrack = BTrackState::Idle;
    s_bSampling = BSamplingState::Idle;
    s_bSmear = BSmearState::Idle;
    s_bStain = BStainState::Idle;
    s_bPrint = BPrintState::Idle;

    m_ProcessMap.clear();

    if (m_isTrackEn) {
        m_ProcessMap.insert("track_reset", false);
    } else {
        m_progress += 15;
    }

    if (m_isPrintEn) {
        m_ProcessMap.insert("print_reset", false);
    } else {
        qInfo() << "Print Module is Forbidden.";
        m_progress += 5;
    }

    m_ProcessMap.insert("sampling_reset", false);
    m_ProcessMap.insert("smear_reset", false);
    m_ProcessMap.insert("stain_reset", false);

#if 0
    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_needle").toBool() == true) {
        m_ProcessMap.insert("sampling_needles", false);
    }
    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_blade").toBool() == true) {
        m_ProcessMap.insert("smear_cleanBlade", false);
    }
#endif
    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_allslots", "enable").toBool() == true) {
        m_ProcessMap.insert("clean_slots", false);
    }
}

void MSleepProcess::onTimer_slot()
{
    if (m_isTrackEn) {
        bootTrackProcess();
    }
    if (m_isPrintEn) {
        bootPrintProcess();
    }
    bootSamplingProcess();
    bootSmearProcess();
    bootStainProcess();

    if (checkStateAllFinished() == true) {
        m_isFinished = true;
        mTimer->stop();

        MLights::GetInstance()->setSlideStoreStop();
        MLights::GetInstance()->setStainOnlyStop();
        TrackManager::GetInstance()->mImport->stopReceiveNewRack();

        sendOkMessage();
        sendFinishSignal();

        DModuleManager::GetInstance()->stopAutoTest();
    } else if (m_isError == true) {
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
    }
}
void MSleepProcess::bootTrackProcess()
{
    if (s_bTrack >= BTrackState::Done) {
        return;
    }

    QString str;

    switch (s_bTrack) {
    case BTrackState::Idle:
        if (m_ProcessMap.contains("track_reset")) {
            if (m_ProcessMap.value("track_reset") == false) {
                s_bTrack = BTrackState::Reset_Device_Machine;
            } else {
                s_bTrack = BTrackState::Finish;
            }
        }
        break;

    case BTrackState::Reset_Device_Machine:
    {
        bool ok = dev->track()->cmd_Reset();
        if (ok) {
            str = "track module init..";
            s_bTrack = BTrackState::WaitF_Reset_Device_Done;
        }
    }
        break;
    case BTrackState::WaitF_Reset_Device_Done:
        if (dev->track()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("track_reset", true);
            m_progress += 15;
            s_bTrack = BTrackState::Finish;
        } else if (dev->track()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bTrack = BTrackState::Error;
        }
        break;

    case BTrackState::Finish:
        s_bTrack = BTrackState::Done;
        break;
    case BTrackState::Error:
        break;
    case BTrackState::Done:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "shutDownProcess", str);
    }
}

/* board #1 */
void MSleepProcess::bootSamplingProcess()
{
    if (s_bSampling >= BSamplingState::Done) {
        return;
    }

    QString str;

    switch (s_bSampling) {
    case BSamplingState::Idle:
        s_bSampling = BSamplingState::Reset_Device_Machine;
        break;

    case BSamplingState::Reset_Device_Machine:
        dev->sample()->cmd_Reset();
        str = "sampling module init..";
        s_bSampling = BSamplingState::WaitF_Reset_Device_Done;
        break;
    case BSamplingState::WaitF_Reset_Device_Done:
        if (dev->sample()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("sampling_reset", true);
            m_progress += 20;
            s_bSampling = BSamplingState::Clean_Needles;
        } else if (dev->sample()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bSampling = BSamplingState::Error;
        }
        break;

    case BSamplingState::Clean_Needles:
        if (m_ProcessMap.contains("sampling_needles")) {
            str = "Start Clean Needles..";
            dev->sample()->cmd_Clean_AddNeedle_Maintain();
            s_bSampling = BSamplingState::WaitF_Clean_Needles_Done;
        } else {
            s_bSampling = BSamplingState::Finish;
        }
        break;
    case BSamplingState::WaitF_Clean_Needles_Done:
        if (dev->sample()->isFuncDone("Clean_AddNeedle_Maintain")) {
            s_bSampling = BSamplingState::Finish;
        }
        break;

    case BSamplingState::Finish:
        m_progress += 5;
        m_ProcessMap.insert("sampling_needles", true);
        s_bSampling = BSamplingState::Done;
        break;
    case BSamplingState::Error:
        break;
    case BSamplingState::Done:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "shutDownProcess", str);
    }
}

/* board #3 */
void MSleepProcess::bootSmearProcess()
{
    if (s_bSmear >= BSmearState::Done) {
        return;
    }

    QString str;

    switch (s_bSmear) {
    case BSmearState::Idle:
        s_bSmear = BSmearState::Reset_Device_Machine;
        break;

    case BSmearState::Reset_Device_Machine:
        if (s_bSampling > BSamplingState::WaitF_Reset_Device_Done) {
            dev->smear()->cmd_Reset();
            str = "smear module init..";
            s_bSmear = BSmearState::WaitF_Reset_Device_Done;
        }
        break;
    case BSmearState::WaitF_Reset_Device_Done:
        if (dev->smear()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("smear_reset", true);
            m_progress += 15;
            s_bSmear = BSmearState::Fill_Blade_Clean_Pool;
        } else if (dev->smear()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bSmear = BSmearState::Error;
        }
        break;

    case BSmearState::Fill_Blade_Clean_Pool:
        if (m_ProcessMap.contains("smear_cleanBlade")) {
            if (s_bSampling == BSamplingState::Finish) {
                dev->smear()->cmd_FillWashPool();
                s_bSmear = BSmearState::WaitF_Fill_Blade_Clean_Pool_Done;
            }
        } else {
            s_bSmear = BSmearState::Finish;
        }
        break;
    case BSmearState::WaitF_Fill_Blade_Clean_Pool_Done:
        if (dev->smear()->isFuncDone("FillWashPool")) {
            s_bSmear = BSmearState::Clean_Blade;
        }
        break;

    case BSmearState::Clean_Blade:
        str = "clean smear blade..";
        dev->smear()->cmd_CleanSmearBlade();
        s_bSmear = BSmearState::WaitF_Clean_Blade_Done;
        break;
    case BSmearState::WaitF_Clean_Blade_Done:
        if (dev->smear()->isFuncDone("CleanSmearBlade")) {
            m_ProcessMap.insert("smear_cleanBlade", true);
            m_progress += 5;
            s_bSmear = BSmearState::Finish;
        }
        break;

    case BSmearState::Finish:
        s_bSmear = BSmearState::Done;
        break;
    case BSmearState::Error:
        break;
    case BSmearState::Done:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "shutDownProcess", str);
    }
}

/* board #2 */
void MSleepProcess::bootStainProcess()
{
    if (s_bStain >= BStainState::Done) {
        return;
    }

    QString str;

    switch (s_bStain) {
    case BStainState::Idle:
        s_bStain = BStainState::Reset_Device_Machine;
        break;

    case BStainState::Reset_Device_Machine:
        dev->stain()->cmd_Reset();
        str = "stain module init..";
        s_bStain = BStainState::WaitF_Reset_Device_Done;
        break;
    case BStainState::WaitF_Reset_Device_Done:
        if (dev->stain()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("stain_reset", true);
            m_progress += 25;

            s_bStain = BStainState::Unload_Recycle_Box;
        } else if (dev->stain()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bStain = BStainState::Error;
        }
        break;

    case BStainState::Unload_Recycle_Box:
    {
        auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
        if (db) {
            QJsonArray arr = db->selectRecord("slot", "group_id='recycle' AND slide_id!=''");
            if (arr.isEmpty() == false) {
                dev->stain()->cmd_RecycleBox_ToDryPos();
                s_bStain = BStainState::WaitF_Unload_Recycle_Box_Done;
                break;
            }
        }
    }
        s_bStain = BStainState::Clean_All_Slots;
        break;
    case BStainState::WaitF_Unload_Recycle_Box_Done:
        if (dev->stain()->isFuncDone("RecycleBox_ToDryPos")) {
            dev->stain()->cmd_RecycleBox_ToManualPos();

            /* record */
            if (mRecordDb) {
                QJsonObject setObj;
                setObj.insert("slide_id", "");
                setObj.insert("slide_uid", "");
                setObj.insert("sample_id", "");
                setObj.insert("sample_uid", "");
                mRecordDb->updateRecord("slot", setObj, {{"group_id", "recycle"}});
            }

            s_bStain = BStainState::Clean_All_Slots;
        }
        break;

    case BStainState::Clean_All_Slots:
        if (m_ProcessMap.contains("clean_slots"))
        {
            /* record */
            if (mRecordDb) {
                QJsonArray arr = mRecordDb->selectRecord("slot", "isSolutionFilled>0");
                if (arr.count() > 0) {
                    SlotsManager::GetInstance()->cleanSlots(arr, "water");
                    str = "clean all slots..";
                    s_bStain = BStainState::WaitF_Clean_All_Slots_Done;
                    break;
                }
            }
        }
        s_bStain = BStainState::Finish;
        break;
    case BStainState::WaitF_Clean_All_Slots_Done:
        if (SlotsManager::GetInstance()->isCleanSlotsFinished()) {
            s_bStain = BStainState::Finish;
        }
        break;

    case BStainState::Finish:
        m_progress += 5;
        m_ProcessMap.insert("clean_slots", true);
        s_bStain = BStainState::Done;
        break;
    case BStainState::Error:
        break;
    case BStainState::Done:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "shutDownProcess", str);
    }
}

void MSleepProcess::bootPrintProcess()
{
    if (s_bPrint >= BPrintState::Done) {
        return;
    }

    QString str;

    switch (s_bPrint) {
    case BPrintState::Idle:
        s_bPrint = BPrintState::Reset_Device_Machine;
        break;

    case BPrintState::Reset_Device_Machine:
        str = "print module init..";
        dev->print()->cmd_Reset();
        s_bPrint = BPrintState::WaitF_Reset_Device_Done;
        break;
    case BPrintState::WaitF_Reset_Device_Done:
        if (dev->print()->getFuncResult("PrinterReset") == Func_Done) {
            m_ProcessMap.insert("print_reset", true);
            m_progress += 5;
            s_bPrint = BPrintState::Finish;
        } else if (dev->print()->getFuncResult("PrinterReset") == Func_Fail) {
            m_isError = true;
            s_bPrint = BPrintState::Error;
        }
        break;

    case BPrintState::Finish:
        s_bPrint = BPrintState::Done;
        break;
    case BPrintState::Error:
        break;
    case BPrintState::Done:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "shutDownProcess", str);
    }
}
