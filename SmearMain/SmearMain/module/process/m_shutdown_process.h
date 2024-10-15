#ifndef M_SHUTDOWN_PROCESS_H
#define M_SHUTDOWN_PROCESS_H

#include "m_process_base.h"

class MShutDownProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MShutDownProcess(QObject *parent = nullptr);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    quint8 m_progress;
    bool m_isUnited;
    bool m_isPrintEn;
    bool m_isTrackEn;

    enum class BTrackState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Finish,
        Error,
        Done
    } s_bTrack;

    enum class BSamplingState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Clean_Needles,
        WaitF_Clean_Needles_Done,

        Finish,
        Error,
        Done
    } s_bSampling;

    enum class BSmearState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Fill_Blade_Clean_Pool,
        WaitF_Fill_Blade_Clean_Pool_Done,

        Clean_Blade,
        WaitF_Clean_Blade_Done,

        Finish,
        Error,
        Done
    } s_bSmear;

    enum class BStainState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Unload_Recycle_Box,
        WaitF_Unload_Recycle_Box_Done,

        Drain_Fix,
        WaitF_Drain_Fix_Done,

        Clean_All_Slots,
        WaitF_Clean_All_Slots_Done,

        Drain_Waste_Tank,
        WaitF_Drain_Waste_Tank_Done,

        Finish,
        Error,
        Done
    } s_bStain;

    enum class BPrintState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Finish,
        Error,
        Done
    } s_bPrint;

    void bootTrackProcess();
    void bootSamplingProcess();
    void bootSmearProcess();
    void bootStainProcess();
    void bootPrintProcess();
};

#endif // M_SHUTDOWN_PROCESS_H
