#include "slot_fix.h"
#include "stain/solution/solution_drainer.h"

SlotFix::SlotFix(int pos, const QString &group, QObject *parent)
    : SlotBase{pos, group, parent}
    , MaxVolume(100)
    , StepVolume(10)
{
    mExpiryTimeSec = 2 * 60 * 60;
    m_isClean = true;
    m_solutionDurationSec = 0;

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout,
            this, &SlotFix::onProcessTimer_slot);

    s_state = Process::Idle;
    m_isDrainFinished = true;
    m_isAutoSolutionFill = FCommon::GetInstance()->getConfigValue("stain", "auto_fill_solution", "auto_fill_fix").toBool();
    qDebug() << mSlotInfo << "Auto Solution Fill:" << m_isAutoSolutionFill;

    m_expiryTimer = new QTimer(this);
    m_expiryTimer->setInterval(1000);   //1s
    connect(m_expiryTimer, &QTimer::timeout,
            this, &SlotFix::onExpiryTimeout_slot);
}

void SlotFix::prepareSolution(const QString &sid)
{
    if (s_state == Process::Idle) {
        m_prepare_slideid = sid;
        m_timer->start();
        qDebug() << QString("%1 prepare Solution, sid:%2").arg(mSlotInfo).arg(sid);
    }
}

void SlotFix::setMaxStainCount(int stainCount)
{
    mMaxStainCount = stainCount;
    qDebug().noquote() << QString("%1 set MaxStainCount: %2")
                          .arg(mSlotInfo).arg(stainCount);
}

void SlotFix::setSolutionExpiryTime(int min)
{
    mExpiryTimeSec = min * 60;
    qDebug().noquote() << QString("%1 set SolutionExpiryTime: %2min")
                          .arg(mSlotInfo).arg(min);
}

bool SlotFix::drainSlot(quint64 id)
{
    if (m_isDrainFinished == true) {
        m_isDrainFinished = false;
        s_state = Process::Drain_Slot;
        if (id > 0) {
            m_request_id = id;
        }
        if (m_timer->isActive() == false) {
            m_timer->start();
        }
        return true;
    }
    return false;
}

bool SlotFix::isAvailable()
{
    if (SlotBase::isAvailable() == false) {
        return false;
    }
    if (m_isClean == false) {
        return false;
    }
    return true;
}

void SlotFix::handlePutinSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("fix_0", QTime::currentTime().toString("HH:mm:ss"));
}

void SlotFix::handleTakeoutSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("fix_1", QTime::currentTime().toString("HH:mm:ss"));
}

bool SlotFix::cmd_Fill_Solution(int volume)
{
    if (mPos == 21) {
        m_api = "Slot_Fix1_Fill";
        return dev->stain()->cmd_Slot_Fix1_Fill(volume);
    } else if (mPos == 22) {
         m_api = "Slot_Fix2_Fill";
        return dev->stain()->cmd_Slot_Fix2_Fill(volume);
    }
    return false;
}

bool SlotFix::cmd_Drain_Slot()
{
    if (mPos == 21) {
        bool ok = dev->stain()->cmd_Slot_Fix1_Drain();
        if (ok) {
            m_api = "Slot_Fix1_Drain";
            mDrainer->addDrainCount(1);
            m_stainCount = 0;
        }
        return ok;
    } else if (mPos == 22) {
        bool ok = dev->stain()->cmd_Slot_Fix2_Drain();
        if (ok) {
            m_api = "Slot_Fix2_Drain";
            mDrainer->addDrainCount(1);
            m_stainCount = 0;
        }
        return ok;
    }
    return false;
}

void SlotFix::onProcessTimer_slot()
{
    switch (s_state) {
    case Process::Idle:
        if (m_isAutoSolutionFill == true
                && m_isSolutionFilled == false
                && m_prepare_slideid.isEmpty() == false
                && m_isClean == true) {
            s_state = Process::Fill_Slot;
        }
        break;

    case Process::Fill_Slot:
    {
        bool ok = cmd_Fill_Solution(MaxVolume);
        if (ok) {
            s_state = Process::WaitF_Fill_Slot_Done;
        }
    }
        break;
    case Process::WaitF_Fill_Slot_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_expiryTimer->start();
            m_stainCount = 0;
            m_solutionDurationSec = 0;
            m_isSolutionFilled = true;
            m_prepare_slideid.clear();

            /* record */
            QJsonObject setObj;
            setObj.insert("isSolutionFilled", 1);
            setObj.insert("stainCount", 0);
            setObj.insert("solution_startTime", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
            mRecordDb->updateRecord("slot", setObj, {{"slot_pos", mPos}});

            s_state = Process::WaitF_Change;
        }
        break;

    case Process::WaitF_Change:
        if (isNeedChange() == true) {
            m_expiryTimer->stop();
            s_state = Process::Drain_Slot;
        }
        break;

#if 0
    case Process::Drain_First:
    {
        bool ok = cmd_Drain_Slot();
        if (ok) {
            s_state = Process::Supply_Solution;
        }
    }
        break;
    case Process::Supply_Solution:
        if (dev->stain()->isFuncDone(m_api)) {
            bool ok = cmd_Fill_Solution(/*StepVolume*/MaxVolume);
            if (ok) {
                s_state = Process::WaitF_Supply_Done;
            }
        }
        break;
    case Process::WaitF_Supply_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_expiryTimer->start();
            m_solutionDurationSec = 0;
            m_isClean = true;

            m_isSolutionFilled = true;
            s_state = Process::WaitF_Change;
        }
        break;
#endif
    case Process::Drain_Slot:
    {
        bool ok = cmd_Drain_Slot();
        if (ok) {
            m_isSolutionFilled = false;
            s_state = Process::WaitF_Drain_Slot_Done;
        }
    }
        break;
    case Process::WaitF_Drain_Slot_Done:
        if (dev->stain()->isFuncDone(m_api)) {
            m_isDrainFinished = true;
            m_stainCount = 0;
            m_solutionDurationSec = 0;
            m_isClean = true;

            /* record */
            QJsonObject setObj;
            setObj.insert("solution_startTime", "");
            setObj.insert("isSolutionFilled", 0);
            setObj.insert("stainCount", 0);
            setObj.insert("isValid", 1);
            mRecordDb->updateRecord("slot", setObj, {{"slot_pos", mPos}});

            s_state = Process::Finish;
        }
        break;

    case Process::Finish:
        s_state = Process::Idle;
        m_timer->stop();

        if (m_request_id > 0) {
            FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, m_request_id);
            m_request_id = 0;
        }
        break;

    default:
        break;
    }
}

bool SlotFix::isNeedSupply()
{
    if (m_stainCount >= mMaxStainCount) {
        qDebug() << "";
        return true;
    }
    return false;
}

bool SlotFix::isNeedChange()
{
    if (m_stainCount >= mMaxStainCount) {
        qDebug() << mSlotInfo << "stain count limit. Need Change.";
        return true;
    } else if (m_isClean == false) {
        qDebug() << mSlotInfo << "solution time limit. Need Change.";
        return true;
    }
    return false;
}

void SlotFix::onExpiryTimeout_slot()
{
    if (m_isSolutionFilled == true) {
        m_solutionDurationSec++;
    }
    if (m_solutionDurationSec >= mExpiryTimeSec) {
        m_expiryTimer->stop();
        m_isClean = false;

        /* record */
        mRecordDb->updateRecord("slot", {{"isValid", 0}}, {{"slot_pos", mPos}});

        qDebug().noquote() << QString("%1 need change, timeout(%2,%3)")
                              .arg(mSlotInfo).arg(m_solutionDurationSec).arg(mExpiryTimeSec);
    }
}
