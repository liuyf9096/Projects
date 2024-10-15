#include "m_clean_all_slots_process.h"
#include "record/rt_machine_record.h"
#include "stain/stain_manager.h"
#include "sql/f_sql_database_manager.h"

MCleanAllSlotsProcess::MCleanAllSlotsProcess(QObject *parent)
    : MProcessBase{"cleanAllSlotsProcess", "cleanSlots", parent}
{
    m_waitTimer = new QTimer(this);
    m_waitTimer->setSingleShot(true);
    m_waitTimer->setInterval(10);
    connect(m_waitTimer, &QTimer::timeout,
            this, &MCleanAllSlotsProcess::onWaitTimer_slot);

    Manager = StainManager::GetInstance()->mSlotManager;

    _init();
}

void MCleanAllSlotsProcess::setDetergentTime(int sec)
{
    m_waitTimer->setInterval(sec * 1000);
}

void MCleanAllSlotsProcess::_init()
{
    m_progress = 0;
    s_clean = CleanAllSlotsState::Idle;
}

void MCleanAllSlotsProcess::state_init()
{
    _init();
}

void MCleanAllSlotsProcess::onTimer_slot()
{
//    process_1();
    process_2();
}

void MCleanAllSlotsProcess::process_1()
{
#if 0
    QString str;

    switch (s_clean) {
    case CleanAllSlotsState::Idle:
        if (m_isFinished == false && m_isError == false) {
            m_progress = 0;
            s_clean = CleanAllSlotsState::Drain_Fix1;
        }
        break;

//! [Fix]
    case CleanAllSlotsState::Drain_Fix1:
        str = "Start Drain Pool-Fix...";
        dev->stain()->cmd_Slot_Fix1_Drain();
        s_clean = CleanAllSlotsState::WaitF_Drain_Fix1_Done;
        break;
    case CleanAllSlotsState::WaitF_Drain_Fix1_Done:
        if (dev->stain()->isFuncDone("Slot_Fix1_Drain")) {
            m_progress += 5;
            s_clean = CleanAllSlotsState::Drain_Fix2;
        }
        break;

    case CleanAllSlotsState::Drain_Fix2:
        dev->stain()->cmd_Slot_Fix2_Drain();
        s_clean = CleanAllSlotsState::WaitF_Drain_Fix2_Done;
        break;
    case CleanAllSlotsState::WaitF_Drain_Fix2_Done:
        if (dev->stain()->isFuncDone("Slot_Fix2_Drain")) {
            m_progress += 5;
            str = "Drain Pool-Fix Finish. Start Clean Remain Slots...";
            s_clean = CleanAllSlotsState::Get_Contaminated_Slots;
        }
        break;
//! [Fix]

    case CleanAllSlotsState::Get_Contaminated_Slots:
    {
        m_handlingPosList.clear();

        auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
        if (db) {
            QJsonArray arr = db->selectRecord("slot", "isSolutionFilled=1 OR isDetergentFilled=1");
            if (arr.count() > 0) {
                for (int i = 0; i < arr.count(); ++i) {
                    QJsonObject obj = arr.at(i).toObject();
                    QString group_id = obj.value("group_id").toString();
                    if (group_id == "c1" || group_id == "c2") {
                        int slot_pos = obj.value("slot_pos").toInt();
                        m_handlingPosList.append(slot_pos);
                    }
                }
            }
        }
        if (m_handlingPosList.isEmpty() == false) {
            s_clean = CleanAllSlotsState::Handle_Slot;
        } else {
            s_clean = CleanAllSlotsState::Finish;
        }
    }
        break;

    case CleanAllSlotsState::Handle_Slot:
        if (m_handlingPosList.isEmpty()) {
            m_handlePos = m_handlingPosList.takeFirst();
            s_clean = CleanAllSlotsState::Drain_Slot_1;
        } else {
            s_clean = CleanAllSlotsState::Finish;
        }
        break;

    case CleanAllSlotsState::Drain_Slot_1:
        StainManager::GetInstance()->mDrainer->addRequest(m_handlePos);
        s_clean = CleanAllSlotsState::WaitF_Drain_1_Done;
        break;
    case CleanAllSlotsState::WaitF_Drain_1_Done:
        if (StainManager::GetInstance()->mDrainer->isIdle()) {
            s_clean = CleanAllSlotsState::Fill_Detergent_Slot;
        }
        break;

    case CleanAllSlotsState::Fill_Detergent_Slot:
        StainManager::GetInstance()->mInfuser->addDetergentRequest("water", m_handlePos);
        s_clean = CleanAllSlotsState::WaitF_Fill_Done;
        break;
    case CleanAllSlotsState::WaitF_Fill_Done:
        if (StainManager::GetInstance()->mInfuser->isIdle()) {
            m_waitTimer->start();
            m_wait = true;
            s_clean = CleanAllSlotsState::Wait_Some_Time;
        }
        break;

    case CleanAllSlotsState::Wait_Some_Time:
        if (m_wait == false) {
            s_clean = CleanAllSlotsState::Drain_Slot_2;
        }
        break;

    case CleanAllSlotsState::Drain_Slot_2:
        StainManager::GetInstance()->mDrainer->addRequest(m_handlePos);
        s_clean = CleanAllSlotsState::WaitF_Drain_2_Done;
        break;
    case CleanAllSlotsState::WaitF_Drain_2_Done:
        if (StainManager::GetInstance()->mDrainer->isIdle()) {
            m_handlePos = -1;
            s_clean = CleanAllSlotsState::Handle_Slot;
        }
        break;

    case CleanAllSlotsState::Finish:
        m_isFinished = true;
        sendOkMessage();
        s_clean = CleanAllSlotsState::Idle;
        break;

    case CleanAllSlotsState::Error:
        m_isError = true;
        sendErrorMessage();
        s_clean = CleanAllSlotsState::Idle;
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "CleanPoolsProcess", str);
    }
#endif
}

void MCleanAllSlotsProcess::process_2()
{
    QString str;

    switch (s_clean) {
    case CleanAllSlotsState::Idle:
        if (m_isFinished == false && m_isError == false) {
            m_progress = 0;
            s_clean = CleanAllSlotsState::Clean_C1_Slots;
        }
        break;

        //! [Fix]
#if 0
    case CleanAllSlotsState::Drain_Fix_Slots:
        str = "Start Drain Pool-Fix...";
        Manager->mGroupFix->cleanAllSlots();
        s_clean = CleanAllSlotsState::WaitF_Drain_Fix_Slots_Done;
        break;
    case CleanAllSlotsState::WaitF_Drain_Fix_Slots_Done:
        if (Manager->mGroupFix->isCleanFinished()) {
            m_progress += 10;
            s_clean = CleanAllSlotsState::Clean_C1_Slots;
        }
        break;
#endif
        //! [C1]
    case CleanAllSlotsState::Clean_C1_Slots:
        Manager->mGroupC1->cleanAllSlots(Detergent_Water);
        s_clean = CleanAllSlotsState::WaitF_Clean_C1_Slots_Done;
        break;
    case CleanAllSlotsState::WaitF_Clean_C1_Slots_Done:
        if (Manager->mGroupC1->isCleanFinished()) {
            m_progress += 30;
            s_clean = CleanAllSlotsState::Clean_C2_Slots;
        }
        break;

        //! [C2]
    case CleanAllSlotsState::Clean_C2_Slots:
        Manager->mGroupC2->cleanAllSlots(Detergent_Water);
        s_clean = CleanAllSlotsState::WaitF_Clean_C2_Slots_Done;
        break;
    case CleanAllSlotsState::WaitF_Clean_C2_Slots_Done:
        if (Manager->mGroupC2->isCleanFinished()) {
            m_progress += 50;
            s_clean = CleanAllSlotsState::Finish;
        }
        break;

    case CleanAllSlotsState::Finish:
        m_isFinished = true;
        sendOkMessage();
        s_clean = CleanAllSlotsState::Idle;
        break;

    case CleanAllSlotsState::Error:
        m_isError = true;
        sendErrorMessage();
        s_clean = CleanAllSlotsState::Idle;
        break;

    default:
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "CleanPoolsProcess", str);
    }
}

void MCleanAllSlotsProcess::onWaitTimer_slot()
{
    m_wait = false;
}
