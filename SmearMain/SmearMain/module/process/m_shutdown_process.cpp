#include "m_shutdown_process.h"
#include "f_common.h"
#include "module_manager.h"
#include "stain/stain_manager.h"
#include "stain/slot/slots_manager.h"

MShutDownProcess::MShutDownProcess(QObject *parent)
    : MProcessBase{"shutdownprocess", "shutDown", parent}
{
    m_isUnited = FCommon::GetInstance()->isUnited();
    m_isTrackEn = FCommon::GetInstance()->isTrackEnable();
    m_isPrintEn = FCommon::GetInstance()->isPrintEnable();

    _init();
}

void MShutDownProcess::state_init()
{
    _init();
}

void MShutDownProcess::_init()
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

    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_needle").toBool() == true) {
        m_ProcessMap.insert("sampling_needles", false);
    }
    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_blade").toBool() == true) {
        m_ProcessMap.insert("smear_cleanBlade", false);
    }
    if (FCommon::GetInstance()->getConfigValue("shutdown", "clean_allslots", "enable").toBool() == true) {
        m_ProcessMap.insert("clean_slots", false);
    }
}

void MShutDownProcess::onTimer_slot()
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
        sendOkMessage();
        sendFinishSignal();

        DModuleManager::GetInstance()->stopAutoTest();
    } else if (m_isError == true) {
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
    }
}
void MShutDownProcess::bootTrackProcess()
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
        qDebug() << "ShutDown Result:" << m_ProcessMap;
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
void MShutDownProcess::bootSamplingProcess()
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
            str = "Start Clean Needles..";
            s_bSampling = BSamplingState::Clean_Needles;
        } else if (dev->sample()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bSampling = BSamplingState::Error;
        }
        break;

    case BSamplingState::Clean_Needles:
        if (m_ProcessMap.contains("sampling_needles")) {
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
        qDebug() << "ShutDown Result:" << m_ProcessMap;
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
void MShutDownProcess::bootSmearProcess()
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
            s_bSmear = BSmearState::Finish;
        }
        break;

    case BSmearState::Finish:
        m_ProcessMap.insert("smear_cleanBlade", true);
        m_progress += 5;
        qDebug() << "ShutDown Result:" << m_ProcessMap;
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
void MShutDownProcess::bootStainProcess()
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
        if (StainManager::GetInstance()->mRecycleBoxes->getCurrentBox() != nullptr) {
            dev->stain()->cmd_RecycleBox_ToDryPos();
            s_bStain = BStainState::WaitF_Unload_Recycle_Box_Done;
        } else {
            s_bStain = BStainState::Clean_All_Slots;
        }
#if 0
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
        s_bStain = BStainState::Clean_All_Slots;
    }
#endif
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

            s_bStain = BStainState::Drain_Fix;
        }
        break;

    case BStainState::Drain_Fix:
        if (mRecordDb) {
            QJsonArray arr = mRecordDb->selectRecord("slot", "isSolutionFilled=1 AND group_id='fix'");
            if (arr.count() > 0) {
                QVector<int> arr1;
                for (int i = 0; i < arr.count(); ++i) {
                    QJsonObject obj = arr.at(i).toObject();
                    int slot_pos = obj.value("slot_pos").toInt();
                    arr1.append(slot_pos);
                }
                if (arr1.count() > 0) {
                    SlotsManager::GetInstance()->mGroupFix->drainSlots(arr1);
                    s_bStain = BStainState::WaitF_Drain_Fix_Done;
                }
                break;
            }
        }
        s_bStain = BStainState::Clean_All_Slots;
        break;
    case BStainState::WaitF_Drain_Fix_Done:
        if (SlotsManager::GetInstance()->mGroupFix->isDrainFinished()) {
            s_bStain = BStainState::Clean_All_Slots;
        }
        break;

    case BStainState::Clean_All_Slots:
        if (m_ProcessMap.contains("clean_slots"))
        {
            /* record */
            if (mRecordDb) {
                QJsonArray arr = mRecordDb->selectRecord("slot", "isUsed>0 AND (group_id='c1' OR group_id='c2')");
                if (arr.count() > 0) {
                    QString detergent = FCommon::GetInstance()->getConfigValue("shutdown", "clean_allslots", "dtergent").toString();
                    SlotsManager::GetInstance()->cleanSlots(arr, detergent);
                    str = "clean all slots..";
                    s_bStain = BStainState::WaitF_Clean_All_Slots_Done;
                    break;
                }
            }
        }
        s_bStain = BStainState::Drain_Waste_Tank;
        break;
    case BStainState::WaitF_Clean_All_Slots_Done:
        if (SlotsManager::GetInstance()->isCleanSlotsFinished()) {
            s_bStain = BStainState::Drain_Waste_Tank;
        }
        break;

    case BStainState::Drain_Waste_Tank:
    {
        bool ok = dev->stain()->cmd_Drain_Waste_Tank();
        if (ok) {
            s_bStain = BStainState::WaitF_Drain_Waste_Tank_Done;
        }
    }
        break;
    case BStainState::WaitF_Drain_Waste_Tank_Done:
        if (dev->stain()->isFuncDone("Drain_Waste_Tank")) {
            s_bStain = BStainState::Finish;
        }
        break;

    case BStainState::Finish:
        m_progress += 5;
        m_ProcessMap.insert("clean_slots", true);
        qDebug() << "ShutDown Result:" << m_ProcessMap;
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

void MShutDownProcess::bootPrintProcess()
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
        qDebug() << "ShutDown Result:" << m_ProcessMap;
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

