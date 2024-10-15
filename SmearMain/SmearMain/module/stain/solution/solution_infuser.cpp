#include "solution_infuser.h"
#include "stain/slot/slots_manager.h"
#include "sql/f_sql_database_manager.h"

#include <QDateTime>

SolutionInfuser::SolutionInfuser(const QString &mid, QObject *parent)
    : DModuleBase{mid, "solutionInfuser", parent}
{
    mTimer = new QTimer(this);
    mTimer->setInterval(50);
    connect(mTimer, &QTimer::timeout,
            this, &SolutionInfuser::onTaskTimer_slot);

    state_init();
}

void SolutionInfuser::state_init()
{
    m_state = Process::Idle;
}

void SolutionInfuser::addRequest(const QString &solution, int pos, int pa, int pb)
{
    QJsonObject obj;
    obj.insert("solution", solution);
    obj.insert("pos", pos);
    obj.insert("pa", pa);
    obj.insert("pb", pb);

    m_requestObjList.append(obj);

    qDebug() << "Solution-Infuser addRequest:" << obj;
}

void SolutionInfuser::addDetergentRequest(const QString &solution, int pos)
{
    QJsonObject obj;
    obj.insert("solution", solution);
    obj.insert("pos", pos);

    m_requestObjList.append(obj);

    qDebug() << "Solution-Infuser addRequest:" << obj;
}

void SolutionInfuser::addDetergentRequest(const QString &solution, QList<int> posList)
{
    for (int i = 0; i < posList.count(); ++i) {
        QJsonObject obj;
        obj.insert("solution", solution);
        obj.insert("pos", posList.at(i));

        qDebug() << "Solution Infuser addRequest:" << obj;

        m_requestObjList.append(obj);
    }
}

void SolutionInfuser::start()
{
    mTimer->start();
    DModuleBase::start();
}

void SolutionInfuser::reset()
{
    state_init();
    DModuleBase::reset();
}

void SolutionInfuser::stop()
{
    mTimer->stop();
    DModuleBase::stop();
}

bool SolutionInfuser::cmd_FillSolution(const QString &solution, int pos, int pa, int pb)
{
    if (solution == "a1") {
        m_api = "Slot_Fill_A1";
        return dev->stain()->cmd_Slot_Fill_A1(pos);
    } else if (solution == "b1") {
        m_api = "Slot_Fill_B1";
        return dev->stain()->cmd_Slot_Fill_B1(pos);
    } else if (solution == "a1b1") {
        m_api = "Slot_Fill_A1B1";
        return dev->stain()->cmd_Slot_Fill_A1B1(pos, pa, pb);
    } else if (solution == "a2") {
        m_api = "Slot_Fill_A1";
        return dev->stain()->cmd_Slot_Fill_A1(pos);
    } else if (solution == "b2") {
        m_api = "Slot_Fill_B2";
        return dev->stain()->cmd_Slot_Fill_B2(pos);
    } else if (solution == "a2b2") {
        m_api = "Slot_Fill_A2B2";
        return dev->stain()->cmd_Slot_Fill_A2B2(pos, pa, pb);
    } else if (solution == "water") {
        m_api = "Slot_Fill_Water";
        return dev->stain()->cmd_Slot_Fill_Water(pos);
    } else if (solution == "alcohol") {
        m_api = "Slot_Fill_Alcohol";
        return dev->stain()->cmd_Slot_Fill_Alcohol(pos);
    }
    return false;
}

void SolutionInfuser::onTaskTimer_slot()
{
    if (dev->stain()->isResetOk() == false) {
        return;
    }

    switch (m_state) {
    case Process::Idle:
        if (m_requestObjList.isEmpty() == false) {
            m_taskObj = m_requestObjList.first();
            m_state = Process::Send_Fill_Cmd;
        }
        break;
    case Process::Send_Fill_Cmd:
    {
        m_solution = m_taskObj.value("solution").toString();
        m_handlingPos = m_taskObj.value("pos").toInt();
        int pa = m_taskObj.value("pa").toInt();
        int pb = m_taskObj.value("pb").toInt();

        bool ok = cmd_FillSolution(m_solution, m_handlingPos, pa, pb);
        if (ok) {
            m_state = Process::WaitF_Fill_Cmd_Done;
        }
    }
        break;
    case Process::WaitF_Fill_Cmd_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_requestObjList.removeFirst();

            handleSlotState(m_api, m_handlingPos);
            m_handlingPos = -1;
            m_solution.clear();
            m_state = Process::Idle;
        }
        break;
    }
}

void SolutionInfuser::handleSlotState(const QString &api, int pos)
{
    auto slot = SlotsManager::GetInstance()->slotAt(pos);
    if (slot) {
        if (api.contains("a1", Qt::CaseInsensitive)
                || api.contains("b1", Qt::CaseInsensitive)) {
            slot->m_isSolutionFilled = true;
        } else {
            slot->m_isDetergentFilled = true;
        }
    }
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
    if (db) {
        if (m_solution == "water") {
            QJsonObject setObj;
            setObj.insert("isDetergentFilled", 1);
            setObj.insert("stainCount", 0);
            setObj.insert("solution_startTime", "");
            db->updateRecord("slot", setObj, {{"slot_pos", pos}});
        } else if (m_solution == "alcohol") {
            QJsonObject setObj;
            setObj.insert("isDetergentFilled", 1);
            setObj.insert("isUsed", 0);
            setObj.insert("stainCount", 0);
            setObj.insert("solution_startTime", "");
            db->updateRecord("slot", setObj, {{"slot_pos", pos}});
        } else {
            QJsonObject setObj;
            setObj.insert("isSolutionFilled", 1);
            setObj.insert("isUsed", 1);
            setObj.insert("stainCount", 0);
            setObj.insert("solution_startTime", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
            db->updateRecord("slot", setObj, {{"slot_pos", pos}});
        }
    }
}
