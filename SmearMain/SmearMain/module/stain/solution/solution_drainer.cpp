#include "solution_drainer.h"
#include "stain/slot/slots_manager.h"
#include "sql/f_sql_database_manager.h"
#include "process/process_manager.h"
#include "f_common.h"

SolutionDrainer::SolutionDrainer(const QString &mid, QObject *parent)
    : DModuleBase{mid, "solutionDrainer", parent}
{
    MaxDrainCount = FCommon::GetInstance()->getConfigValue("stain", "drainer", "max_drain_slot_count").toInt();
    qDebug() << "[Drainer] max drain slot count:" << MaxDrainCount;
    Q_ASSERT(MaxDrainCount > 0);

    m_floaterAlarmValue = 1;
    m_checkFloaterAlarm = FCommon::GetInstance()->getConfigValue("stain", "drainer", "floater_alarm_force_drain").toBool();
    if (m_checkFloaterAlarm == true) {
        QJsonArray arr = FCommon::GetInstance()->getConfigValue("check", "sample", "floater").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject obj = arr.at(i).toObject();
            QString floater_id = obj.value("id").toString();
            int alarm_value = obj.value("alarm_value").toInt();
            if (floater_id == "WasteTank_Full") {
                m_floaterAlarmValue = alarm_value;
            }
        }
        qDebug() << "SolutionDrainer::m_tankAlarmValue" << m_floaterAlarmValue;
    }

    mDrainSlotTimer = new QTimer(this);
    mDrainSlotTimer->setInterval(50);
    connect(mDrainSlotTimer, &QTimer::timeout,
            this, &SolutionDrainer::onDrainSlotTimer_slot);

    int drain_tank = FCommon::GetInstance()->getConfigValue("stain", "drainer", "auto_drain_tank_min").toInt();
    Q_ASSERT(drain_tank > 0);
    qDebug() << "[Drainer] Auto Drain Tank Time:" << drain_tank << "min";

    m_fixDrainTimer = new QTimer(this);
    m_fixDrainTimer->setInterval(drain_tank * 60 * 1000);
    connect(m_fixDrainTimer, &QTimer::timeout,
            this, &SolutionDrainer::onDrainWasteTankTimer_slot);

    state_init();
}

void SolutionDrainer::state_init()
{
    m_isCleanTank = false;
    m_drainCount = 0;
    s_drainslot = DrainSlot::Idle;
    s_drainwaste = DrainWaste::Idle;
}

void SolutionDrainer::addRequest(int pos)
{
    m_requestList.append(pos);
    qDebug() << "Solution-Drainer addRequest:" << pos;
}

void SolutionDrainer::addRequest(QList<int> posList)
{
    m_requestList.append(posList);
    qDebug() << "Solution-Drainer addRequest:" << posList;
}

void SolutionDrainer::addDrainCount(int count)
{
    m_drainCount = m_drainCount + count;
    qDebug() << "add drainCount:" << m_drainCount;

    if (m_drainCount >= MaxDrainCount) {
        qDebug() << "It's time to drain waste tank"
                 << "drainCount:" << m_drainCount
                 << "Max:" << MaxDrainCount;
    }
}

void SolutionDrainer::start()
{
    mDrainSlotTimer->start();
    m_fixDrainTimer->start();
    DModuleBase::start();
}

void SolutionDrainer::reset()
{
    state_init();
    DModuleBase::reset();
}

void SolutionDrainer::stop()
{
    mDrainSlotTimer->stop();
    m_fixDrainTimer->stop();
    DModuleBase::stop();
}

void SolutionDrainer::onDrainSlotTimer_slot()
{
    if (dev->stain()->isResetOk() == false) {
        return;
    }

    if (m_checkFloaterAlarm
            && dev->stain()->checkFloaterSensorValue("WasteTank_Full") == m_floaterAlarmValue) {
        qWarning() << "WasteTank_Full" << "Need drain waste tank!";
        m_isCleanTank = true;
    }

    if (m_drainCount >= MaxDrainCount || m_isCleanTank) {
        drainTankProcess();
    } else {
        drainSlotProcess();
    }
}

void SolutionDrainer::drainSlotProcess()
{
    switch (s_drainslot) {
    case DrainSlot::Idle:
        if (m_requestList.isEmpty() == false) {
            m_handlingPos = m_requestList.first();
            s_drainslot = DrainSlot::Send_Drain_Cmd;
        }
        break;
    case DrainSlot::Send_Drain_Cmd:
    {
        bool ok = dev->stain()->cmd_Slot_Drain_WithUp(m_handlingPos);
        if (ok) {
            s_drainslot = DrainSlot::WaitF_Drain_Cmd_Done;
        }
    }
        break;
    case DrainSlot::WaitF_Drain_Cmd_Done:
        if (dev->stain()->isFuncDone("Slot_Drain_WithUp")) {
            m_requestList.removeFirst();

            auto slot = SlotsManager::GetInstance()->slotAt(m_handlingPos);
            if (slot) {
                slot->m_isDrained = true;
                slot->m_stainCount = 0;
            }
            auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
            if (db) {
                QJsonObject setObj;
                setObj.insert("isSolutionFilled", 0);
                setObj.insert("isDetergentFilled", 0);
                setObj.insert("stainCount", 0);
                setObj.insert("solution_startTime", "");
                db->updateRecord("slot", setObj, {{"slot_pos", m_handlingPos}});
            }

            m_handlingPos = -1;
            m_drainCount++;
            s_drainslot = DrainSlot::Idle;
        }
        break;
    }
}

void SolutionDrainer::drainTankProcess()
{
    switch (s_drainwaste) {
    case DrainWaste::Idle:
        s_drainwaste = DrainWaste::Send_Drain_Tank_Cmd;
        break;
    case DrainWaste::Send_Drain_Tank_Cmd:
    {
        bool ok = dev->stain()->cmd_Drain_Waste_Tank();
        if (ok) {
            s_drainwaste = DrainWaste::Reset_Drain_Needle;
        }
    }
        break;
    case DrainWaste::Reset_Drain_Needle:
    {
        bool ok = dev->stain()->cmd_Drain_Needle_Reset();
        if (ok) {
            m_fixDrainTimer->start();
            s_drainwaste = DrainWaste::WaitF_Drain_Tank_Done;
        }
    }
        break;

    case DrainWaste::WaitF_Drain_Tank_Done:
        if (dev->stain()->isFuncDone("Drain_Waste_Tank")
                && dev->stain()->isFuncDone("Drain_Needle_Reset")) {
            m_drainCount = 0;
            m_isCleanTank = false;
            s_drainwaste = DrainWaste::Idle;
        }
        break;
    }
}

void SolutionDrainer::onDrainWasteTankTimer_slot()
{
    qDebug() << "It's time to drain waste tank. Times up.";
    m_isCleanTank = true;
}

