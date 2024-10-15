#ifndef M_STAIN_AGING_PROCESS_H
#define M_STAIN_AGING_PROCESS_H

#include "m_process_base.h"

class MStainAgingProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MStainAgingProcess(QObject *parent = nullptr);

    virtual bool startProcess(quint64 id) override;
    virtual void resetProcess(quint64 id) override;

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    int pos;
    int r_pos;

    int m_doneCount;

    enum State {
        Idle,

        State1,
        State2,

        Prepare_1,
        Prepare_2,

        From_Import_To_Fix_1,
        From_Import_To_Fix_2,
        From_Import_To_Fix_3,
        From_Import_To_Fix_4,
        From_Import_To_Fix_5,

        From_Fix_To_A1_1,
        From_Fix_To_A1_2,
        From_Fix_To_A1_3,
        From_Fix_To_A1_4,
        From_Fix_To_A1_5,

        From_A1_To_Tr_1,
        From_A1_To_Tr_2,
        From_A1_To_Tr_3,
        From_A1_To_Tr_4,

        From_Tr_To_Wash_1,
        From_Tr_To_Wash_2,
        From_Tr_To_Wash_3,
        From_Tr_To_Wash_4,

        From_Wash_To_Recycle_1,
        From_Wash_To_Recycle_2,
        From_Wash_To_Recycle_3,
        From_Wash_To_Recycle_4,
        From_Wash_To_Recycle_5,

        Renew_Slide_1,
        Renew_Slide_2,
        Renew_Slide_3,
        Renew_Slide_4,
        Renew_Slide_5,
        Renew_Slide_6,

        Finish
    } m_state;

    void process2();
};

#endif // M_STAIN_AGING_PROCESS_H
