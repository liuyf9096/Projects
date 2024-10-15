#ifndef M_REMOVE_REMAIN_STAIN_SLIDES_H
#define M_REMOVE_REMAIN_STAIN_SLIDES_H

#include "m_process_base.h"

class MRemoveRemainStainSlides : public MProcessBase
{
    Q_OBJECT
public:
    explicit MRemoveRemainStainSlides(QObject *parent = nullptr);

protected slots:
    virtual void onTimer_slot() override;

protected:
    virtual void state_init() override;

    enum class RemoveState {
        Idle,

        Get_New_RecycleBox,
        WaitF_New_RecycleBox_Done,

        Assign_Remain_Slides,

        Remove,
        WaitF_Remove_Done,

        Remove_RecycleBox,
        WaitF_Remove_RecycleBox_Done,

        Finish,
        Error
    } s_rm;

private:
    QJsonArray m_remainSlidesArr;

    QJsonArray handleRemainSlides();
};

#endif // M_REMOVE_REMAIN_STAIN_SLIDES_H
