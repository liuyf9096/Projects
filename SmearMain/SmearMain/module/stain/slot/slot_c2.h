#ifndef SLOT_C2_H
#define SLOT_C2_H

#include "slot_base.h"

class SolutionInfuser;
class SolutionDrainer;
class SlotC2 : public SlotBase
{
    friend class SlotGroupC2;

    Q_OBJECT
public:
    explicit SlotC2(int pos, const QString &group, QObject *parent = nullptr);

    void setSolutionRate(int a, int b);
    virtual void setMaxStainCount(int stainCount) override;
    virtual void setSolutionExpiryTime(int min) override;

    bool drainSlot(quint64 id = 0);
    bool isDrainFinished() { return m_isDrainFinished; }

    bool cleanSlot(Detergent d, quint64 id = 0);
    bool isCleanFinished() { return m_isCleanFinished; }

    virtual bool isAvailable() override;
    virtual void prepareSolution(const QString &sid) override;

protected:
    void setInfuser(SolutionInfuser *infuser) { mInfuser = infuser; }
    void setDrainer(SolutionDrainer *drainer) { mDrainer = drainer; }

    virtual void handlePutinSlide() override;
    virtual void handleTakeoutSlide() override;

private:
    SolutionInfuser *mInfuser;
    SolutionDrainer *mDrainer;
    int pa, pb;

    bool m_isCleanFinished;
    int m_wait;
    int m_waitMax;
    Detergent m_detergent;

    enum class Process {
        Idle,

        Fill_Solution,
        WaitF_Fill_Solution_Done,

        WaitF_Solution_Lost_Effect,

        Drain_Waste,
        WaitF_Drain_Waste_Done,

        Fill_Detergent,
        WaitF_Fill_Detergent_Done,

        Wait_Some_Time,

        Drain_Detergent,
        WaitF_Drain_Detergent_Done,

        Finish
    } s_state;

    QTimer *m_expiryTimer;
    QTimer *m_processTimer;

    QTimer *m_drainTimer;
    bool m_isDrainFinished;
    enum class DrainProcess {
        Idle,

        Drain_Slot,
        WaitF_Drain_Slot_Done,

        Finish
    } s_drain;

private slots:
    void onExpiryTimeout_slot();
    void onSolutionProcess_slot();
    void onDrainProcess_slot();
};

#endif // SLOT_C2_H
