#ifndef M_BOOT_PROCESS_H
#define M_BOOT_PROCESS_H

#include "module_base.h"

class MBootProcess : public DModuleBase
{
    Q_OBJECT
public:
    explicit MBootProcess(QObject *parent = nullptr);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();

    QTimer *m_bootTimer;
    enum class BootState {
        Idle,

        Syn_Time_1,
        WaitF_Syn_Time_1_Done,

        Syn_Time_2,
        WaitF_Syn_Time_2_Done,

        Reset_Track_1,
        WaitF_Reset1_Done,

        Reset_Track_2,
        WaitF_Reset2_Done,

        Finish
    } s_boot;
    bool m_bootFinished;

private slots:
    void onBootTimer_slot();
};

#endif // M_BOOT_PROCESS_H
