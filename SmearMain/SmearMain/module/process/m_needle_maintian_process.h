#ifndef M_NEEDLE_MAINTAIN_PROCESS_H
#define M_NEEDLE_MAINTAIN_PROCESS_H

#include "m_process_base.h"

class MNeedleMaintainProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MNeedleMaintainProcess(QObject *parent = nullptr);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();
    int m_progress;

    enum class PState {
        Idle,

        Reset_Closet,
        WaitF_Reset_Closet_Done,

        Get_Maintian_Tube,
        WaitF_Get_Tube_Done,

        Place_Tube_To_Cart,
        WaitF_Place_Tube_Done,

        Move_Tube_To_Take_Pos,
        WaitF_Move_Tube_Done,

        Soak_Needle_Maintain,
        WaitF_Soak_Needle_Done,

        Wash_Pool_Drain_Open1,
        WaitF_Wash_Pool_Drain_Open_Done1,

        Take_Solution_To_Blade_Pool,
        WaitF_Take_Solution_Done,

        Wash_Pool_Drain_Close1,
        WaitF_Wash_Pool_Drain_Close_Done1,

        Move_Tube_To_Exit,
        WaitF_Move_Tube_To_Exit_Done,

        Return_Tube_To_Closet,
        WaitF_Return_Tube_Done,

        Push_Out_Closet,
        WaitF_Push_Out_Closet_Done,

        Perfuse_Maintain_Blade_Pool,
        WaitF_Perfuse_Maintain_Blade_Pool_Done,

        Wash_Needle_Maintian,
        WaitF_Wash_Needle_Done,

        Wash_Blade_Maintian,
        WaitF_Wash_Blade_Done,

        Wash_Pool_Drain_Open2,
        WaitF_Wash_Pool_Drain_Open_Done2,

        Perfuse_Diluent_Tube,
        WaitF_Perfuse_Diluent_Tube_Done,

        Wash_Pool_Drain_Close2,
        WaitF_Wash_Pool_Drain_Close_Done2,

        Finish,
        Error
    } m_state;

    void maintianProcess();
};

#endif // M_NEEDLE_MAINTAIN_PROCESS_H
