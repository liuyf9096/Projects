#include "m_needle_maintian_process.h"
#include "module_manager.h"
#include "record/rt_machine_record.h"
#include "sql/f_sql_database_manager.h"
#include "module/process/process_manager.h"
#include "f_common.h"

#include <QDateTime>

MNeedleMaintainProcess::MNeedleMaintainProcess(QObject *parent)
    : MProcessBase{"needleprocess", "needleprocess", parent}
{
    _init();
}

void MNeedleMaintainProcess::_init()
{
    m_isFinished = true;
    m_state = PState::Idle;
    m_progress = 0;
}

void MNeedleMaintainProcess::state_init()
{
    _init();
}

void MNeedleMaintainProcess::onTimer_slot()
{
    maintianProcess();
}

void MNeedleMaintainProcess::maintianProcess()
{
    QString str;

    switch (m_state) {
    case PState::Idle:
        if (m_isFinished == false) {
            m_state = PState::Reset_Closet;
        }
        break;

    case PState::Reset_Closet:
        dev->track()->cmd_Emergency_Close();
        m_state = PState::WaitF_Reset_Closet_Done;
        break;
    case PState::WaitF_Reset_Closet_Done:
        if (dev->track()->isFuncDone("Emergency_Close")) {
            m_state = PState::Get_Maintian_Tube;
        }
        break;

    case PState::Get_Maintian_Tube:
        dev->sample()->cmd_GetNewTube(1);
        m_state = PState::WaitF_Get_Tube_Done;
        break;
    case PState::WaitF_Get_Tube_Done:
        if (dev->sample()->isFuncDone("GetNewTube")) {
            m_state = PState::Place_Tube_To_Cart;
        }
        break;

    case PState::Place_Tube_To_Cart:
        dev->sample()->cmd_TubeFrGripToCart();
        m_state = PState::WaitF_Place_Tube_Done;
        break;
    case PState::WaitF_Place_Tube_Done:
        if (dev->sample()->isFuncDone("TubeFrGripToCart")) {
            m_state = PState::Move_Tube_To_Take_Pos;
        }
        break;

    case PState::Move_Tube_To_Take_Pos:
        dev->sample()->cmd_CartToSamplePos();
        m_state = PState::WaitF_Move_Tube_Done;
        break;
    case PState::WaitF_Move_Tube_Done:
        if (dev->sample()->isFuncDone("CartToSamplePos")) {
            m_state = PState::Take_Solution_To_Blade_Pool;
        }
        break;

    case PState::Soak_Needle_Maintain:
        dev->sample()->cmd_Soak_Needle_Maintain();
        m_state = PState::WaitF_Soak_Needle_Done;
        break;
    case PState::WaitF_Soak_Needle_Done:
        if (dev->sample()->isFuncDone("Soak_Needle_Maintain")) {
             m_state = PState::Wash_Pool_Drain_Open1;
        }
        break;

    case PState::Wash_Pool_Drain_Open1:
        dev->smear()->cmd_WashPoolDrain_Open();
        m_state = PState::WaitF_Wash_Pool_Drain_Open_Done1;
        break;

    case PState::WaitF_Wash_Pool_Drain_Open_Done1:
        if (dev->smear()->isFuncDone("WashPoolDrain_Open")) {
            m_state = PState::Take_Solution_To_Blade_Pool;
        }
        break;

    case PState::Take_Solution_To_Blade_Pool:
        dev->sample()->cmd_NeedleAdd_TakeSample_To_BladePool();
        m_state = PState::WaitF_Take_Solution_Done;
        break;

    case PState::WaitF_Take_Solution_Done:
        if (dev->sample()->isFuncDone("NeedleAdd_TakeSample_To_BladePool")) {
            m_state = PState::Wash_Pool_Drain_Close1;
        }
        break;

    case PState::Wash_Pool_Drain_Close1:
        dev->smear()->cmd_WashPoolDrain_Close();
        m_state = PState::WaitF_Wash_Pool_Drain_Close_Done1;
        break;
    case PState::WaitF_Wash_Pool_Drain_Close_Done1:
        if (dev->smear()->isFuncDone("WashPoolDrain_Close")) {
            m_state = PState::Move_Tube_To_Exit;
        }
        break;

    case PState::Move_Tube_To_Exit:
        dev->sample()->cmd_CartToExitPos();
        m_state = PState::WaitF_Move_Tube_To_Exit_Done;
        break;
    case PState::WaitF_Move_Tube_To_Exit_Done:
        if (dev->sample()->isFuncDone("CartToExitPos")) {
            m_state = PState::Return_Tube_To_Closet;
        }
        break;

    case PState::Return_Tube_To_Closet:
        dev->sample()->cmd_ReturnTubeFrCart(1);
        m_state = PState::WaitF_Return_Tube_Done;
        break;
    case PState::WaitF_Return_Tube_Done:
        if (dev->sample()->isFuncDone("ReturnTubeFrCart")) {
            m_state = PState::Push_Out_Closet;
        }
        break;

    case PState::Push_Out_Closet:
        dev->track()->cmd_Emergency_Open();
        m_state = PState::WaitF_Push_Out_Closet_Done;
        break;
    case PState::WaitF_Push_Out_Closet_Done:
        if (dev->track()->isFuncDone("Emergency_Open")) {
            m_state = PState::Perfuse_Maintain_Blade_Pool;
        }
        break;

    case PState::Perfuse_Maintain_Blade_Pool:
        dev->sample()->cmd_Perfuse_Maintain_Blade_Pool();
        m_state = PState::WaitF_Perfuse_Maintain_Blade_Pool_Done;
        break;

    case PState::WaitF_Perfuse_Maintain_Blade_Pool_Done:
        if (dev->sample()->isFuncDone("Perfuse_Maintain_Blade_Pool")) {
            m_state = PState::Wash_Needle_Maintian;
        }
        break;

    case PState::Wash_Needle_Maintian:
        dev->sample()->cmd_Clean_AddNeedle_Maintain();
        m_state = PState::WaitF_Wash_Needle_Done;
        break;
    case PState::WaitF_Wash_Needle_Done:
        if (dev->sample()->isFuncDone("Clean_AddNeedle_Maintain")) {
            m_state = PState::Wash_Blade_Maintian;
        }
        break;

    case PState::Wash_Blade_Maintian:
        dev->smear()->cmd_CleanSmearBlade_Maintian();
        m_state = PState::WaitF_Wash_Blade_Done;
        break;
    case PState::WaitF_Wash_Blade_Done:
        if (dev->smear()->isFuncDone("CleanSmearBlade_Maintian")) {
            m_state = PState::Wash_Pool_Drain_Open2;
        }
        break;

    case PState::Wash_Pool_Drain_Open2:
        dev->smear()->cmd_WashPoolDrain_Open();
        m_state = PState::WaitF_Wash_Pool_Drain_Open_Done2;
        break;

    case PState::WaitF_Wash_Pool_Drain_Open_Done2:
        if (dev->smear()->isFuncDone("WashPoolDrain_Open")) {
            m_state = PState::Perfuse_Diluent_Tube;
        }
        break;

    case PState::Perfuse_Diluent_Tube:
        dev->sample()->cmd_Perfuse_Diluent_Tube();
        m_state = PState::WaitF_Perfuse_Diluent_Tube_Done;
        break;

    case PState::WaitF_Perfuse_Diluent_Tube_Done:
        if (dev->sample()->isFuncDone("Perfuse_Diluent_Tube")) {
            m_state = PState::Wash_Pool_Drain_Close2;
        }
        break;

    case PState::Wash_Pool_Drain_Close2:
        dev->smear()->cmd_WashPoolDrain_Close();
        m_state = PState::WaitF_Wash_Pool_Drain_Close_Done2;
        break;

    case PState::WaitF_Wash_Pool_Drain_Close_Done2:
        if (dev->smear()->isFuncDone("WashPoolDrain_Close")) {
            m_state = PState::Finish;
        }
        break;

    case PState::Finish:
        m_isFinished = true;
        mTimer->stop();
        sendOkMessage();
        break;
    case PState::Error:
        m_isFinished = true;
        mTimer->stop();
        sendErrorMessage();
        break;
    }

    if (str.isEmpty() == false) {
        sendProcessMessage(m_progress, "NeedleMaintainProcess", str);
    }
}
