#ifndef M_DRAIN_FILTER_PROCESS_H
#define M_DRAIN_FILTER_PROCESS_H

#include "module_base.h"

class MDrainFilterProcess : public DModuleBase
{
    Q_OBJECT
public:
    explicit MDrainFilterProcess(QObject *parent = nullptr);

protected:
    void state_init();

private:
    void _init();

    QTimer *m_timer;
    QTimer *mStateTimer;
    bool m_isDrainFinished;
    enum class DrainState {
        Idle,
        DrainFilter,
        WaitF_Drain_Filter_Done,
        Finish
    } s_drain;

private slots:
    void onTimer_slot();
    void onDrainTimer_slot();
};

#endif // M_DRAIN_FILTER_PROCESS_H
