#ifndef M_BOOT_PROCESS_H
#define M_BOOT_PROCESS_H

#include "m_process_base.h"

class MBootProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MBootProcess(QObject *parent = nullptr);

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
    int m_wait;

    enum class BTrackState {
        Idle,

        Syn_Time,
        WaitF_Syn_Time_Done,
        Check_Scanning_Reset,
        Wait_Samplg_Reset_Done,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Finish,
        Error
    } s_bTrack;

    enum class BSamplingState {
        Idle,

        Syn_Time,
        WaitF_Syn_Time_Done,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Open_WashPool_Drain,
        WaitF_Open_WashPool_Drain_Done,

        Clean_Liquid_System,
        WaitF_Clean_System_Done,

        Close_WashPool_Drain,
        WaitF_Close_WashPool_Drain_Done,

        Clean_Needles,
        WaitF_Clean_Needles_Done,

        Finish,
        Error
    } s_bSampling;

    enum class BSmearState {
        Idle,

        Syn_Time,
        WaitF_Syn_Time_Done,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Clean_Liquid_System,
        WaitF_Clean_System_Done,

        Fill_Blade_Clean_Pool,
        WaitF_Fill_Blade_Clean_Pool_Done,

        Clean_Blade,
        WaitF_Clean_Blade_Done,

        Finish,
        Error
    } s_bSmear;

    enum class BStainState {
        Idle,

        Syn_Time,
        WaitF_Syn_Time_Done,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Check_Remain_RecycleBox,
        WaitF_Remove_Remain_RecycleBox_Done,

        Clean_Liquid_System,
        WaitF_Clean_System_Done,

        Drain_Waste_Tank,
        WaitF_Drain_Waste_Tank_Done,

        Finish,
        Error
    } s_bStain;

    enum class BPrintState {
        Idle,

        Reset_Device_Machine,
        WaitF_Reset_Device_Done,

        Finish,
        Error
    } s_bPrint;

    void bootTrackProcess();
    void bootSamplingProcess();
    void bootSmearProcess();
    void bootStainProcess();
    void bootPrintProcess();

    void checkException();
};

#endif // M_BOOT_PROCESS_H
