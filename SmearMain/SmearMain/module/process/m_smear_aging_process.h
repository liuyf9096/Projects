#ifndef M_SMEAR_AGING_PROCESS_H
#define M_SMEAR_AGING_PROCESS_H

#include "m_process_base.h"

class MSmearAgingProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MSmearAgingProcess(QObject *parent = nullptr);

    virtual bool startProcess(quint64 id) override;
    virtual void resetProcess(quint64 id) override;

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    int m_doneCount;

    enum State {
        Idle,

        State1,
        State2,

        Get_New_Tube_1,
        Get_New_Tube_2,

        Mix_Tube_1,
        Mix_Tube_2,

        Sampling_1,
        Sampling_2,
        Sampling_3,
        Sampling_4,
        Sampling_5,
        Sampling_6,

        Smear_1,
        Smear_2,
        Smear_3,
        Smear_4,
        Smear_5,
        Smear_6,

        Recycle_Tube_1,
        Recycle_Tube_2,

        Finish
    } m_state;

    void process1();
    void process2();
};

#endif // M_SMEAR_AGING_PROCESS_H
