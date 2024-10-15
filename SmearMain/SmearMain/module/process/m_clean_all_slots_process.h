#ifndef M_CLEAN_ALL_SLOTS_PROCESS_H
#define M_CLEAN_ALL_SLOTS_PROCESS_H

#include "m_process_base.h"

class SlotsManager;
class MCleanAllSlotsProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MCleanAllSlotsProcess(QObject *parent = nullptr);

    void setDetergentTime(int sec);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    quint8 m_progress;

    QList<int> m_handlingPosList;
    int m_handlePos;
    int m_wcount;

    void process_1();
    void process_2();

#if 0
    enum class CleanAllSlotsState {
        Idle,

        Drain_Fix1,
        WaitF_Drain_Fix1_Done,

        Drain_Fix2,
        WaitF_Drain_Fix2_Done,

        Get_Contaminated_Slots,

        Handle_Slot,

        Drain_Slot_1,
        WaitF_Drain_1_Done,

        Fill_Detergent_Slot,
        WaitF_Fill_Done,

        Wait_Some_Time,

        Drain_Slot_2,
        WaitF_Drain_2_Done,

        Finish,
        Error
    } s_clean;
#endif

    enum class CleanAllSlotsState {
        Idle,

        Drain_Fix_Slots,
        WaitF_Drain_Fix_Slots_Done,

        Clean_C1_Slots,
        WaitF_Clean_C1_Slots_Done,

        Clean_C2_Slots,
        WaitF_Clean_C2_Slots_Done,

        Finish,
        Error
    } s_clean;

    QTimer *m_waitTimer;
    bool m_wait;

    SlotsManager *Manager;
private slots:
    void onWaitTimer_slot();
};

#endif // M_CLEAN_ALL_SLOTS_PROCESS_H
