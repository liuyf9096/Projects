#include "slot_c1.h"
#include "stain/solution/solution_infuser.h"
#include "stain/solution/solution_drainer.h"
#include <QDebug>

SlotC1::SlotC1(int pos, const QString &group, QObject *parent)
    : SlotBase{pos, group, parent}
{
    pa = 100;
    pb = 0;

    mExpiryTimeSec = 6 * 60 * 60;
    m_isClean = true;
    m_stainCount = 0;
    s_state = Process::Idle;
    m_solutionDurationSec = 0;

    m_isDrainFinished = true;
    m_isCleanFinished = true;
    m_isAutoSolutionFill = FCommon::GetInstance()->getConfigValue("stain", "auto_fill_solution", "auto_fill_c1").toBool();
    qDebug() << mSlotInfo << "Auto Solution Fill:" << m_isAutoSolutionFill;

    m_wait = 0;
    m_waitMax = 0;
    m_request_id = 0;
    m_detergent = Detergent_Water;

    m_expiryTimer = new QTimer(this);
    m_expiryTimer->setInterval(1000);   //1s
    connect(m_expiryTimer, &QTimer::timeout,
            this, &SlotC1::onExpiryTimeout_slot);

    m_processTimer = new QTimer(this);
    m_processTimer->setInterval(100);
    connect(m_processTimer, &QTimer::timeout,
            this, &SlotC1::onSolutionProcess_slot);

    s_drain = DrainProcess::Idle;
    m_drainTimer = new QTimer(this);
    m_drainTimer->setInterval(100);
    connect(m_drainTimer, &QTimer::timeout,
            this, &SlotC1::onDrainProcess_slot);
}

void SlotC1::setSolutionRate(int a, int b)
{
    pa = a;
    pb = b;
    qDebug().noquote() << QString("%1 set Solution: a:%2% b:%3%")
                          .arg(mSlotInfo).arg(a).arg(b);
}

void SlotC1::setMaxStainCount(int stainCount)
{
    mMaxStainCount = stainCount;
    qDebug().noquote() << QString("%1 set MaxStainCount: %2")
                          .arg(mSlotInfo).arg(stainCount);
}

void SlotC1::setSolutionExpiryTime(int min)
{
    mExpiryTimeSec = min * 60;
    qDebug().noquote() << QString("%1 set SolutionExpiryTime: %2min")
                          .arg(mSlotInfo).arg(min);
}

bool SlotC1::drainSlot(quint64 id)
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

bool SlotC1::cleanSlot(Detergent d, quint64 id)
{
    if (s_state < Process::Drain_Waste /* && m_slide == nullptr */) {
        s_state = Process::Drain_Waste;

        m_detergent = d;
        if (m_detergent == Detergent_Water) {
            m_waitMax = m_water_soaktime_s * 10;   /* soak * 10 * (100) = soak(msec) */
        } else if (m_detergent == Detergent_Alcohol) {
            m_waitMax = m_alcohol_soaktime_s * 10;
        }
        m_isCleanFinished = false;
        m_processTimer->start();
        qDebug().noquote() << QString("%1 clean Slot, pos:%2, soak time: %3sec")
                              .arg(mSlotInfo).arg(mPos).arg(m_waitMax/10);

        if (id > 0) {
            m_request_id = id;
        }
        return true;
    }
    return false;
}

// 1s
void SlotC1::onExpiryTimeout_slot()
{
    if (m_isSolutionFilled == true) {
        m_solutionDurationSec++;
    }
    if (m_solutionDurationSec >= mExpiryTimeSec) {
        m_isClean = false;

        /* record */
        mRecordDb->updateRecord("slot", {{"isValid", 0}}, {{"slot_pos", mPos}});

        m_expiryTimer->stop();
        qDebug().noquote() << QString("%1 need clean, timeout(%2,%3)")
                              .arg(mSlotInfo).arg(m_solutionDurationSec).arg(mExpiryTimeSec);
    }
}

bool SlotC1::isAvailable()
{
    if (SlotBase::isAvailable() == false) {
        return false;
    }
    if (m_isClean == false) {
        return false;
    }
    return true;
}

void SlotC1::prepareSolution(const QString &sid)
{
    if (s_state == Process::Idle) {
        m_prepare_slideid = sid;
        m_processTimer->start();
        qDebug() << QString("%1 prepare Solution, sid:%2").arg(mSlotInfo).arg(sid);
    }
}

void SlotC1::handlePutinSlide()
{
    /* record */
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("c1_0", QTime::currentTime().toString("HH:mm:ss"));
}

void SlotC1::handleTakeoutSlide()
{
    if (m_stainCount >= mMaxStainCount) {
        m_isClean = false;
        m_expiryTimer->stop();
        qDebug().noquote() << QString("%1 need clean, stain count:(%2)")
                              .arg(mSlotInfo).arg(m_stainCount);
    }

    /* record */
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("c1_1", QTime::currentTime().toString("HH:mm:ss"));
}

void SlotC1::onSolutionProcess_slot()
{
    switch (s_state) {
    case Process::Idle:
        if (m_isAutoSolutionFill && m_isSolutionFilled == false && m_prepare_slideid.isEmpty() == false) {
            qDebug() << QString("%1 Slot Prepare Start.").arg(mSlotInfo);
            s_state = Process::Fill_Solution;
        }
        break;

    case Process::Fill_Solution:
        mInfuser->addRequest("a1", mPos, pa, pb);
        s_state = Process::WaitF_Fill_Solution_Done;
        break;
    case Process::WaitF_Fill_Solution_Done:
        if (m_isSolutionFilled == true) {
            /* init */
            m_isClean = true;
            m_stainCount = 0;
            m_solutionDurationSec = 0;
            m_expiryTimer->start();

            /* record */
            mRecordDb->updateRecord("slot", {{"isValid", 1}}, {{"slot_pos", mPos}});
            s_state = Process::WaitF_Solution_Lost_Effect;
        }
        break;

    case Process::WaitF_Solution_Lost_Effect:
        if (m_isClean == false) {
            m_expiryTimer->stop();
            s_state = Process::Drain_Waste;
        }
        break;

    case Process::Drain_Waste:
        m_isCleaning = true;
        m_isDrained = false;
        mDrainer->addRequest(mPos);
        s_state = Process::WaitF_Drain_Waste_Done;
        break;
    case Process::WaitF_Drain_Waste_Done:
        if (m_isDrained == true) {
            m_prepare_slideid.clear();
            m_isSolutionFilled = false;
            s_state = Process::Fill_Detergent;
        }
        break;

    case Process::Fill_Detergent:
        m_isDetergentFilled = false;
        if (m_detergent == Detergent_Water) {
            mInfuser->addRequest("water", mPos);
        } else if (m_detergent == Detergent_Alcohol) {
            mInfuser->addRequest("alcohol", mPos);
        }
        s_state = Process::WaitF_Fill_Detergent_Done;
        break;
    case Process::WaitF_Fill_Detergent_Done:
        if (m_isDetergentFilled == true) {
            m_wait = m_waitMax;
            s_state = Process::Wait_Some_Time;
        }
        break;

    case Process::Wait_Some_Time:
        if (m_wait > 0) {
            m_wait--;
        } else {
            s_state = Process::Drain_Detergent;
        }
        break;

    case Process::Drain_Detergent:
        m_isDrained = false;
        mDrainer->addRequest(mPos);
        s_state = Process::WaitF_Drain_Detergent_Done;
        break;
    case Process::WaitF_Drain_Detergent_Done:
        if (m_isDrained == true) {
            m_isCleanFinished = true;
            m_isCleaning = false;
            m_solutionDurationSec = 0;
            s_state = Process::Finish;
        }
        break;

    case Process::Finish:
        m_isClean = true;
        m_expiryTimer->stop();
        m_processTimer->stop();
        s_state = Process::Idle;

        qDebug() << QString("%1 Slot Maintain End.").arg(mSlotInfo);
        break;
    }
}

void SlotC1::onDrainProcess_slot()
{
    switch (s_drain) {
    case DrainProcess::Idle:
        if (m_isDrainFinished == false) {
            s_drain = DrainProcess::Drain_Slot;
        }
        break;
    case DrainProcess::Drain_Slot:
        m_stainCount = 0;
        m_solutionDurationSec = 0;
        m_isSolutionFilled = false;
        m_isDetergentFilled = false;
        m_isDrained = false;
        mDrainer->addRequest(mPos);
        s_drain = DrainProcess::WaitF_Drain_Slot_Done;
        break;
    case DrainProcess::WaitF_Drain_Slot_Done:
        if (m_isDrained == true) {
            /* record */
            mRecordDb->updateRecord("slot", {{"isSolutionFilled", 0}}, {{"slot_pos", mPos}});
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
