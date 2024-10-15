#include "slot_wash.h"

#define FillBeforePutin

SlotWash::SlotWash(int pos, const QString &group, QObject *parent)
    : SlotBase{pos, group, parent}
{
    m_washCount = 0;
    m_washCountMax = 1;
    m_isDrainFinished = true;

    m_washTimer = new QTimer(this);
    m_washTimer->setInterval(100);
    connect(m_washTimer, &QTimer::timeout,
            this, &SlotWash::onProcessTimer_slot);

    s_state = Process::Idle;

    s_drain = DrainProcess::Idle;
    m_drainTimer = new QTimer(this);
    m_drainTimer->setInterval(100);
    connect(m_drainTimer, &QTimer::timeout,
            this, &SlotWash::onDrainProcess_slot);
}

void SlotWash::setWashCount(int count)
{
    m_washCountMax = count;
    qDebug() << "Set SlotWash Wash Count:" << count;
}

bool SlotWash::drainSlot(quint64 id)
{
    if (m_isDrainFinished == true) {
        m_isDrainFinished = false;
        m_drainTimer->start();
        s_drain = DrainProcess::Idle;
        if (id > 0) {
            m_request_id = id;
        }
        return true;
    }
    return false;
}

void SlotWash::prepareSolution(const QString &sid)
{
    if (s_state == Process::Idle) {
        m_prepare_slideid = sid;
        m_washTimer->start();
        qDebug() << QString("%1 prepare Water, sid:%2").arg(mSlotInfo).arg(sid);
    }
}

void SlotWash::handlePutinSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("wash_0", QTime::currentTime().toString("HH:mm:ss"));

    if (m_washTimer->isActive() == false) {
        m_washTimer->start();
        s_state = Process::Idle;
    }
}

void SlotWash::handleTakeoutSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("wash_1", QTime::currentTime().toString("HH:mm:ss"));

    m_washTimer->stop();
    s_state = Process::Idle;
}

void SlotWash::onDurationTimeout()
{
    m_washTimer->stop();
    s_state = Process::Idle;
}

void SlotWash::onProcessTimer_slot()
{
    switch (s_state) {
    case Process::Idle:
        if (m_prepare_slideid.isEmpty() == false) {
#ifdef FillBeforePutin
            s_state = Process::Fill_Slot;
#else
            s_state = Process::WaitF_Slot_In;
#endif
        }
        break;

#ifdef FillBeforePutin
    case Process::Fill_Slot:
    {
        bool ok = cmd_Slot_Fill();
        if (ok) {
            s_state = Process::WaitF_Fill_Slot_Done;
        }
    }
        break;
    case Process::WaitF_Fill_Slot_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            s_state = Process::WaitF_Slot_In;
        }
        break;
#endif
    case Process::WaitF_Slot_In:
        if (m_slide != nullptr) {
            m_prepare_slideid.clear();

#ifdef FillBeforePutin
            s_state = Process::Drain_Slot;
#else
            s_state = Process::Send_Wash_Cmd;
#endif
        }
        break;

#ifdef FillBeforePutin
    case Process::Drain_Slot:
    {
        bool ok = cmd_Slot_Drain();
        if (ok) {
            s_state = Process::WaitF_Drain_Slot_Done;
        }
    }
        break;
    case Process::WaitF_Drain_Slot_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_washCount = 0;
            s_state = Process::Send_Wash_Cmd;
        }
        break;
#endif

    case Process::Send_Wash_Cmd:
        if (m_washCount < m_washCountMax) {
            bool ok = cmd_Slot_Wash();
            if (ok) {
                s_state = Process::WaitF_Wash_Done;
            }
        } else {
            s_state = Process::Finish;
        }
        break;
    case Process::WaitF_Wash_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_washCount++;
            s_state = Process::Send_Wash_Cmd;
        }
        break;

    case Process::Finish:
        m_washCount = 0;
        m_washTimer->stop();
        SlotBase::onDurationTimeout_slot();
        s_state = Process::Idle;
        break;

    default:
        break;
    }
}

bool SlotWash::cmd_Slot_Wash()
{
    bool ok = false;
    if (mPos == 51) {
        ok = dev->stain()->cmd_Slot_Wash1_Wash();
        if (ok) {
            m_api = "Slot_Wash1_Wash";
        }
    } else if (mPos == 52) {
        ok = dev->stain()->cmd_Slot_Wash2_Wash();
        if (ok) {
            m_api = "Slot_Wash2_Wash";
        }
    }
    return ok;
}

bool SlotWash::cmd_Slot_Fill()
{
    bool ok = false;
    if (mPos == 51) {
        ok = dev->stain()->cmd_Slot_Wash1_Fill();
        if (ok) {
            m_api = "Slot_Wash1_Fill";
        }
    } else if (mPos == 52) {
        ok = dev->stain()->cmd_Slot_Wash2_Fill();
        if (ok) {
            m_api = "Slot_Wash2_Fill";
        }
    }
    return ok;
}

bool SlotWash::cmd_Slot_Drain()
{
    bool ok = false;
    if (mPos == 51) {
        ok = dev->stain()->cmd_Slot_Wash1_Drain();
        if (ok) {
            m_api = "Slot_Wash1_Drain";
        }
    } else if (mPos == 52) {
        ok = dev->stain()->cmd_Slot_Wash2_Drain();
        if (ok) {
            m_api = "Slot_Wash2_Drain";
        }
    }
    return ok;
}

bool SlotWash::isSlotDrainFinished()
{
    if (mPos == 51) {
        return dev->stain()->isFuncDone("Slot_Wash1_Drain");
    } else if (mPos == 52) {
        return dev->stain()->isFuncDone("Slot_Wash2_Drain");
    }
    return false;
}

void SlotWash::onDrainProcess_slot()
{
    switch (s_drain) {
    case DrainProcess::Idle:
        if (m_isDrainFinished == false) {
            s_drain = DrainProcess::Drain_Slot;
        }
        break;
    case DrainProcess::Drain_Slot:
        cmd_Slot_Drain();
        s_drain = DrainProcess::WaitF_Drain_Slot_Done;
        break;
    case DrainProcess::WaitF_Drain_Slot_Done:
        if (isSlotDrainFinished() == true) {
            /* record */
            QJsonObject setObj;
            setObj.insert("isSolutionFilled", 0);
            setObj.insert("isDetergentFilled", 0);
            mRecordDb->updateRecord("slot", setObj, {{"slot_pos", mPos}});

            s_drain = DrainProcess::Finish;
        }
        break;
    case DrainProcess::Finish:
        m_isDrainFinished = true;
        m_drainTimer->stop();
        s_drain = DrainProcess::Idle;

        if (m_request_id > 0) {
            FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, m_request_id);
            m_request_id = 0;
        }
        break;
    }
}
