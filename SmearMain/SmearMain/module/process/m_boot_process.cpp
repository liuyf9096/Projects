#include "m_boot_process.h"
#include "module_manager.h"
#include "record/rt_machine_record.h"
#include "sql/f_sql_database_manager.h"
#include "module/process/process_manager.h"
#include "light/m_lights.h"
#include "f_common.h"

#include <QDateTime>

MBootProcess::MBootProcess(QObject *parent)
    : MProcessBase{"bootprocess", "boot", parent}
{
    m_wait = 0;

    m_isUnited = FCommon::GetInstance()->isUnited();
    m_isTrackEn = FCommon::GetInstance()->isTrackEnable();
    m_isPrintEn = FCommon::GetInstance()->isPrintEnable();

    _init();
}

void MBootProcess::_init()
{
    m_progress = 0;

    s_bTrack = BTrackState::Idle;
    s_bSampling = BSamplingState::Idle;
    s_bSmear = BSmearState::Idle;
    s_bStain = BStainState::Idle;
    s_bPrint = BPrintState::Idle;

    m_ProcessMap.clear();
}

void MBootProcess::state_init()
{
    _init();

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

    QString last_perfuse_time;
    auto arr = mRecordDb->selectRecord("perfuse", QJsonObject({{"item", "last_perfuse_time"}}));
    if (arr.count() > 0) {
        QJsonObject dtobj = arr.first().toObject();
        last_perfuse_time = dtobj.value("datetime").toString();
    }

    QJsonObject obj = FCommon::GetInstance()->getConfigValue("startup", "auto_perfuse").toObject();
    bool enable = obj.value("enable").toBool();
    bool perfuse_sampling = obj.value("perfuse_sampling").toBool();
    bool perfuse_stain = obj.value("perfuse_stain").toBool();
    QDateTime last_dt = QDateTime::fromString(last_perfuse_time, "yyyy-MM-dd hh:mm:ss");
    int max_duration_hr = obj.value("max_duration_hr").toInt();
    qint64 delta_sec = last_dt.secsTo(QDateTime::currentDateTime());
    int duration_min = delta_sec / 60;
    if (enable) {
        qDebug() << "last perfuse time:" << last_dt.toString() << "current time:" << QDateTime::currentDateTime().toString();
        qDebug() << "max duration hr:" << max_duration_hr << "current duration:" << duration_min << "min";
        if (last_perfuse_time.isEmpty() || duration_min > max_duration_hr * 60) {
            qDebug() << "perfuse_sampling" << perfuse_sampling << "perfuse_stain" << perfuse_stain;
            if (perfuse_sampling == true) {
                m_ProcessMap.insert("sampling_cleanliquid", false);
                m_ProcessMap.insert("smear_cleanliquid", false);
                m_ProcessMap.insert("smear_cleanBlade", false);
            }
            if (perfuse_stain == true) {
                m_ProcessMap.insert("stain_perfuse", false);
            }
        }
    }
}

void MBootProcess::onTimer_slot()
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

        MLights::GetInstance()->start();

        sendFinishSignal();
        sendOkMessage();
        checkException();
    } else if (m_isError == true) {
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
    }
}

void MBootProcess::checkException()
{
    QJsonArray arr = mRecordDb->selectRecord("slot", "slide_id!='' AND group_id!='import' AND group_id!='StainOnly' AND group_id!='recycle'");
    if (arr.count() > 0) {
        Exception e("StainSlideRemain_101", E_Level::Error, 101);
        e.e_msg = "Detect many slides remian in the slots, please remove them first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    arr = mRecordDb->selectRecord("slot", "(isSolutionFilled=1 OR isDetergentFilled=1) AND group_id!='import'");
    if (arr.count() > 0) {
        Exception e("StainSlot_Not_Clean", E_Level::Alarm, 102);
        e.e_msg = "Stain Slot remain fluid, please clean first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    arr = mRecordDb->selectRecord("doing_sample", "module='mix' AND sid!=''");
    if (arr.count() > 0) {
        Exception e("MixRemainTube_101", E_Level::Error, 101);
        e.e_msg = "Detect a mixing tube remains inside, please remove it first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    arr = mRecordDb->selectRecord("doing_sample", "module='cart' AND sid!=''");
    if (arr.count() > 0) {
        Exception e("SamplingCartRemainTube_101", E_Level::Error, 101);
        e.e_msg = "Detect a sampling tube remains inside, please remove it first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    arr = mRecordDb->selectRecord("doing_slide", "module='smear_cart' AND slide_id!=''");
    if (arr.count() > 0) {
        Exception e("SmearCartRemainSlide_101", E_Level::Error, 101);
        e.e_msg = "Detect a slide remains in the cart, please remove it first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    arr = mRecordDb->selectRecord("doing_slide", "module='stain_cart' AND slide_id!=''");
    if (arr.count() > 0) {
        Exception e("StainCartRemainSlide_101", E_Level::Error, 101);
        e.e_msg = "Detect a slide remains in the cart, please remove it first.";
        ExceptionCenter::GetInstance()->sendExceptionMessage(e);
    }
    MProcessManager::GetInstance()->mCheckSensorsProc->startCheck();
}

void MBootProcess::bootTrackProcess()
{
    QString str;

    switch (s_bTrack) {
    case BTrackState::Idle:
        if (m_ProcessMap.contains("track_reset")) {
            if (m_ProcessMap.value("track_reset") == false) {
                s_bTrack = BTrackState::Syn_Time;
            } else {
                qDebug() << "Boot Result:" << m_ProcessMap;
                s_bTrack = BTrackState::Finish;
            }
        } else {
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bTrack = BTrackState::Finish;
        }
        break;

    case BTrackState::Syn_Time:
        dev->track()->cmd_SystemTimeSync();
        s_bTrack = BTrackState::WaitF_Syn_Time_Done;
        break;
    case BTrackState::WaitF_Syn_Time_Done:
        if (dev->track()->isFuncDone("SystemTimeSync")) {
            s_bTrack = BTrackState::Check_Scanning_Reset;
        }
        break;

    case BTrackState::Check_Scanning_Reset:
    {
        QJsonArray arr = mRecordDb->selectRecord("doing_sample", "module=='scan' AND slide_id!=''");
        if (arr.count() > 0) {
            s_bTrack = BTrackState::Wait_Samplg_Reset_Done;
            break;
        }
        s_bTrack = BTrackState::Reset_Device_Machine;
    }
        break;

    case BTrackState::Wait_Samplg_Reset_Done:
        if (s_bSampling > BSamplingState::WaitF_Reset_Device_Done) {
            s_bTrack = BTrackState::Reset_Device_Machine;
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
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bTrack = BTrackState::Finish;
        } else if (dev->track()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bTrack = BTrackState::Error;
        }
        break;

    case BTrackState::Finish:
        break;
    case BTrackState::Error:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "BootProcess", str);
    }
}

/* board #1 */
void MBootProcess::bootSamplingProcess()
{
    QString str;

    switch (s_bSampling) {
    case BSamplingState::Idle:
        s_bSampling = BSamplingState::Syn_Time;
        break;

    case BSamplingState::Syn_Time:
        dev->sample()->cmd_SystemTimeSync();
        s_bSampling = BSamplingState::WaitF_Syn_Time_Done;
        break;
    case BSamplingState::WaitF_Syn_Time_Done:
        if (dev->sample()->isFuncDone("SystemTimeSync")) {
            m_wait = 0;
            s_bSampling = BSamplingState::Reset_Device_Machine;
        }
        break;

    case BSamplingState::Reset_Device_Machine:
        m_wait++;
        if (m_wait > 50) {
            dev->sample()->cmd_Reset();
            str = "sampling module init..";
            s_bSampling = BSamplingState::WaitF_Reset_Device_Done;
        }
        break;
    case BSamplingState::WaitF_Reset_Device_Done:
        if (dev->sample()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("sampling_reset", true);
            m_progress += 10;
            s_bSampling = BSamplingState::Open_WashPool_Drain;
        } else if (dev->sample()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bSampling = BSamplingState::Error;
        }
        break;

    case BSamplingState::Open_WashPool_Drain:
        if (m_ProcessMap.contains("sampling_cleanliquid")) {
            if (m_ProcessMap.value("smear_cleanliquid") == true) {
                dev->smear()->cmd_WashPoolDrain_Open();
                s_bSampling = BSamplingState::WaitF_Open_WashPool_Drain_Done;
            }
        } else {
            m_progress += 10;
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bSampling = BSamplingState::Finish;
        }
        break;
    case BSamplingState::WaitF_Open_WashPool_Drain_Done:
        if (dev->smear()->isFuncDone("WashPoolDrain_Open")) {
            s_bSampling = BSamplingState::Clean_Liquid_System;
        }
        break;

    case BSamplingState::Clean_Liquid_System:
        str = "sampling liquid system init..";
        dev->sample()->cmd_CleanLiquidSystem();
        s_bSampling = BSamplingState::WaitF_Clean_System_Done;
        break;
    case BSamplingState::WaitF_Clean_System_Done:
        if (dev->sample()->getFuncResult("CleanLiquidSystem") == Func_Done) {
            m_ProcessMap.insert("sampling_cleanliquid", true);
            m_progress += 5;
            s_bSampling = BSamplingState::Close_WashPool_Drain;
        } else if (dev->sample()->getFuncResult("CleanLiquidSystem") == Func_Fail) {
            m_isError = false;
            s_bSampling = BSamplingState::Error;
        }
        break;

    case BSamplingState::Close_WashPool_Drain:
        dev->smear()->cmd_WashPoolDrain_Close();
        s_bSampling = BSamplingState::WaitF_Close_WashPool_Drain_Done;
        break;
    case BSamplingState::WaitF_Close_WashPool_Drain_Done:
        if (dev->smear()->isFuncDone("WashPoolDrain_Close")) {
            s_bSampling = BSamplingState::Clean_Needles;
        }
        break;

    case BSamplingState::Clean_Needles:
        str = "sampling module clean needle..";
        dev->sample()->cmd_Clean_AddNeedle_Maintain();
        s_bSampling = BSamplingState::WaitF_Clean_Needles_Done;
        break;
    case BSamplingState::WaitF_Clean_Needles_Done:
        if (dev->sample()->isFuncDone("Clean_AddNeedle_Maintain")) {
            str = "sampling module init finish OK.";
            m_progress += 5;
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bSampling = BSamplingState::Finish;
        }
        break;

    case BSamplingState::Finish:
        break;
    case BSamplingState::Error:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "BootProcess", str);
    }
}

/* board #3 */
void MBootProcess::bootSmearProcess()
{
    QString str;

    switch (s_bSmear) {
    case BSmearState::Idle:
        s_bSmear = BSmearState::Syn_Time;
        break;

    case BSmearState::Syn_Time:
        dev->smear()->cmd_SystemTimeSync();
        s_bSmear = BSmearState::WaitF_Syn_Time_Done;
        break;
    case BSmearState::WaitF_Syn_Time_Done:
        if (dev->smear()->isFuncDone("SystemTimeSync")) {
            s_bSmear = BSmearState::Reset_Device_Machine;
        }
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
            m_progress += 10;
            s_bSmear = BSmearState::Clean_Liquid_System;
        } else if (dev->smear()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bSmear = BSmearState::Error;
        }
        break;

    case BSmearState::Clean_Liquid_System:
        if (m_ProcessMap.contains("smear_cleanliquid")) {
            dev->smear()->cmd_CleanLiquidSystem();
            str = "smear liquid system init..";
            s_bSmear = BSmearState::WaitF_Clean_System_Done;
        } else {
            m_progress += 10;
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bSmear = BSmearState::Finish;
        }
        break;
    case BSmearState::WaitF_Clean_System_Done:
        if (dev->smear()->getFuncResult("CleanLiquidSystem") == Func_Done) {
            m_ProcessMap.insert("smear_cleanliquid", true);
            m_progress += 5;
            s_bSmear = BSmearState::Fill_Blade_Clean_Pool;
        } else if (dev->smear()->getFuncResult("CleanLiquidSystem") == Func_Fail) {
            m_isError = false;
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
            qDebug() << "Boot Result:" << m_ProcessMap;
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
            str = "smear module init finish OK.";
            m_progress += 5;
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bSmear = BSmearState::Finish;
        }
        break;

    case BSmearState::Finish:
        break;
    case BSmearState::Error:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "BootProcess", str);
    }
}

/* board #2 */
void MBootProcess::bootStainProcess()
{
    QString str;

    switch (s_bStain) {
    case BStainState::Idle:
        s_bStain = BStainState::Syn_Time;
        break;

    case BStainState::Syn_Time:
        dev->stain()->cmd_SystemTimeSync();
        s_bStain = BStainState::WaitF_Syn_Time_Done;
        break;
    case BStainState::WaitF_Syn_Time_Done:
        if (dev->stain()->isFuncDone("SystemTimeSync")) {
            s_bStain = BStainState::Reset_Device_Machine;
        }
        break;

    case BStainState::Reset_Device_Machine:
        dev->stain()->cmd_Reset();
        str = "stain module init..";
        s_bStain = BStainState::WaitF_Reset_Device_Done;
        break;
    case BStainState::WaitF_Reset_Device_Done:
        if (dev->stain()->getFuncResult("Reset") == Func_Done) {
            m_ProcessMap.insert("stain_reset", true);
            m_progress += 15;

            s_bStain = BStainState::Check_Remain_RecycleBox;
        } else if (dev->stain()->getFuncResult("Reset") == Func_Fail) {
            m_isError = true;
            s_bStain = BStainState::Error;
        }
        break;

    case BStainState::Check_Remain_RecycleBox:
    {
#if 1
        QJsonArray arr = mRecordDb->selectRecord("slot", "group_id='recycle' AND slide_id!=''");
        if (arr.isEmpty() == false) {
            dev->stain()->cmd_RecycleBox_ToDryPos();
            s_bStain = BStainState::WaitF_Remove_Remain_RecycleBox_Done;
            break;
        }
#else
        dev->stain()->cmd_RecycleBox_ToDryPos();
        s_bStain = BStainState::WaitF_Remove_Remain_RecycleBox_Done;
        break;
#endif
    }
        s_bStain = BStainState::Clean_Liquid_System;
        break;
    case BStainState::WaitF_Remove_Remain_RecycleBox_Done:
        if (dev->stain()->isFuncDone("RecycleBox_ToDryPos")) {
            dev->stain()->cmd_RecycleBox_ToManualPos();

            QJsonObject setObj;
            setObj.insert("slide_id", "");
            setObj.insert("slide_uid", "");
            setObj.insert("sample_id", "");
            setObj.insert("sample_uid", "");
            mRecordDb->updateRecord("slot", setObj, {{"group_id", "recycle"}});

            s_bStain = BStainState::Clean_Liquid_System;
        }
        break;

    case BStainState::Clean_Liquid_System:
        if (m_ProcessMap.contains("stain_perfuse")) {
            dev->stain()->cmd_Perfuse_LiquidSystem();
            str = "clean stain liquid system..";
            s_bStain = BStainState::WaitF_Clean_System_Done;
        } else {
            m_progress += 20;
            s_bStain = BStainState::Drain_Waste_Tank;
        }
        break;
    case BStainState::WaitF_Clean_System_Done:
        if (dev->stain()->getFuncResult("Perfuse_LiquidSystem") == Func_Done) {
            m_ProcessMap.insert("stain_perfuse", true);

            /* record */
            QString last = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            mRecordDb->updateRecord("perfuse", {{"datetime", last}}, {{"item", "last_perfuse_time"}});

            m_progress += 20;
            s_bStain = BStainState::Drain_Waste_Tank;
        } else if (dev->stain()->getFuncResult("Perfuse_LiquidSystem") == Func_Fail) {
            m_isError = false;
            s_bStain = BStainState::Error;
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
            str = "stain module init finish OK.";
            m_progress += 5;
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bStain = BStainState::Finish;
        }
        break;

    case BStainState::Finish:
        break;
    case BStainState::Error:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "BootProcess", str);
    }
}

void MBootProcess::bootPrintProcess()
{
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
            qDebug() << "Boot Result:" << m_ProcessMap;
            s_bPrint = BPrintState::Finish;
        } else if (dev->print()->getFuncResult("PrinterReset") == Func_Fail) {
            m_isError = true;
            s_bPrint = BPrintState::Error;
        }
        break;

    case BPrintState::Finish:
        break;
    case BPrintState::Error:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "BootProcess", str);
    }
}

