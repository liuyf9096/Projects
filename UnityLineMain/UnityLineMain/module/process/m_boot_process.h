#ifndef M_BOOT_PROCESS_H
#define M_BOOT_PROCESS_H

#include "module_base.h"

class QTimer;
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

        Syn_Time,
        WaitF_Syn_Time_Done,

        Check_Exit_Full,
        WaitF_Check_Res_Done,

        Reset_Track,
        WaitF_Reset_Done,

        Check_DMU_Connect_State,

        Error,
        Finish
    } s_boot;
    bool m_bootFinished;

private slots:
    void onBootTimer_slot();
};

#endif // M_BOOT_PROCESS_H
