#ifndef M_CLEAN_LIQUID_SYSTEM_PROCESS_H
#define M_CLEAN_LIQUID_SYSTEM_PROCESS_H

#include "m_process_base.h"

class MCleanLiquidSystemProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MCleanLiquidSystemProcess(QObject *parent = nullptr);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    enum CleanLiquidState {
        Idle,

        Prepare,
        Prepare_Done,

        Clean_Liquid_System,
        WaitF_Clean_System_Done,

        Closing,
        Closing_Done,

        Finish,
        Error
    };

    CleanLiquidState s_Sampling;
    CleanLiquidState s_Smear;
    CleanLiquidState s_Stain;

    void cleanSamplingProcess();
    void cleanSmearProcess();
    void cleanStainProcess();
};

#endif // M_CLEAN_LIQUID_SYSTEM_PROCESS_H
