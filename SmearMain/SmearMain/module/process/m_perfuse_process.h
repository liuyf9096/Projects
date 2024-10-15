#ifndef M_PERFUSE_PROCESS_H
#define M_PERFUSE_PROCESS_H

#include "m_process_base.h"

class MPerfuseProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MPerfuseProcess(QObject *parent = nullptr);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    enum class PerfuseState {
        Idle,

        Perfuse_Water,
        WaitF_Perfuse_Water_Done,

        Perfuse_Fix,
        WaitF_Perfuse_Fix_Done,

        Clean_After_Perfuse_Fix,
        WaitF_Clean_After_Perfuse_Fix_Down,

        Perfuse_A1,
        WaitF_Perfuse_A1_Done,

        Perfuse_B1,
        WaitF_Perfuse_B1_Done,

        Perfuse_A2,
        WaitF_Perfuse_A2_Done,

        Perfuse_B2,
        WaitF_Perfuse_B2_Done,

        Clean_After_Perfuse,
        WaitF_Clean_Done,

        Finish,
        Error
    } s_perfuse;

    int m_wait;
};

#endif // M_PERFUSE_PROCESS_H
