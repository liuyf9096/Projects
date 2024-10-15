#ifndef M_REMOVE_REMAIN_TUBE_H
#define M_REMOVE_REMAIN_TUBE_H

#include "m_process_base.h"

class MEmergency;
class MRemoveRemainTube : public MProcessBase
{
    Q_OBJECT
public:
    enum TubePosition {InCart, InMix, InGrip, Unknown};

    explicit MRemoveRemainTube(QObject *parent = nullptr);
    void startProcess(TubePosition pos, quint64 id);
    void setEmergency(MEmergency *emerg) { mEmerg = emerg; }

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    MEmergency *mEmerg = nullptr;
    TubePosition m_pos;

    void _init();

    void removeCartTube();
    void removeMixTube();
    void removeGripTube();

    int m_waitTime;
    enum class RemoveState {
        Idle,

        Reset_Closet,
        WaitF_Reset_Closet_Done,

        Move_To_Cap_Detect,
        WaitF_Move_To_Detect_Done,

        Check_Cap_Status,
        WaitF_Check_Res,

        Cap_Tube,
        WaitF_Cap_Tube_Done,

        Remove_Last_Tube,
        WaitF_Remove_Last_Tube_Done,

        Open_Closet,
        WaitF_Open_Closet_Done,

        WaitF_Sometime_Done,

        Close_Closet,
        WaitF_Close_Closet_Done,

        Finish
    } s_removeCart, s_removeMix, s_removeGrip;
    QString m_api;
};

#endif // M_REMOVE_REMAIN_TUBE_H
