#include "m_drain_filter_process.h"

MDrainFilterProcess::MDrainFilterProcess(QObject *parent)
    : DModuleBase{"drainfilterprocess", "drainfilter", parent}
{
    _init();

    m_timer = new QTimer(this);
    m_timer->setInterval(30 * 60 * 1000);
    connect(m_timer, &QTimer::timeout,
            this, &MDrainFilterProcess::onTimer_slot);

    mStateTimer = new QTimer(this);
    mStateTimer->setInterval(100);
    connect(mStateTimer, &QTimer::timeout,
            this, &MDrainFilterProcess::onDrainTimer_slot);

//    m_timer->start();
}

void MDrainFilterProcess::_init()
{
    s_drain = DrainState::Idle;
    m_isDrainFinished = true;
}

void MDrainFilterProcess::state_init()
{
    _init();
}

void MDrainFilterProcess::onTimer_slot()
{
    if (m_isDrainFinished == true) {
        m_isDrainFinished = false;
        s_drain = DrainState::Idle;
        if (mStateTimer->isActive() == false) {
            mStateTimer->start();
        }
    }
}

void MDrainFilterProcess::onDrainTimer_slot()
{
    switch (s_drain) {
    case DrainState::Idle:
        if (m_isDrainFinished == false) {
            s_drain = DrainState::DrainFilter;
        }
        break;
    case DrainState::DrainFilter:
    {
        bool ok = dev->stain()->cmd_Drain_Waste_Tank();
        if (ok) {
            s_drain = DrainState::WaitF_Drain_Filter_Done;
        }
    }
        break;
    case DrainState::WaitF_Drain_Filter_Done:
        if (dev->stain()->isFuncDone("Drain_Waste_Tank")) {
            m_isDrainFinished = true;
            s_drain = DrainState::Finish;
        }
        break;
    case DrainState::Finish:
        break;
    }
}
