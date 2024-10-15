#ifndef M_RESET_PROCESS_H
#define M_RESET_PROCESS_H

#include "m_process_base.h"

class MResetProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MResetProcess(QObject *parent = nullptr);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    enum class ResetState {
        Idle,
        Reset_Device_Machine,
        WaitF_Reset_Device_Done,
        Finish,
        Error
    };

    ResetState s_rstTrask;
    ResetState s_rstSampling;
    ResetState s_rstSmear;
    ResetState s_rstStain;
    ResetState s_rstPrint;

    void resetTrackProcess();
    void resetSamplingProcess();
    void resetSmearProcess();
    void resetStainProcess();
    void resetPrintProcess();
};

#endif // M_RESET_PROCESS_H
