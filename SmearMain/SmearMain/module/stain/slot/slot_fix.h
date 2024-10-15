#ifndef SLOT_FIX_H
#define SLOT_FIX_H

#include "slot_base.h"

class SolutionDrainer;
class SlotFix : public SlotBase
{
    friend class SlotGroupFix;
    Q_OBJECT
public:
    explicit SlotFix(int pos, const QString &group, QObject *parent = nullptr);

    virtual void prepareSolution(const QString &sid) override;

    virtual void setMaxStainCount(int stainCount) override;
    virtual void setSolutionExpiryTime(int min) override;

    bool drainSlot(quint64 id = 0);
    bool isDrainFinished() { return m_isDrainFinished; }

    virtual bool isAvailable() override;

protected:
    void setDrainer(SolutionDrainer *drainer) { mDrainer = drainer; }
    virtual void handlePutinSlide() override;
    virtual void handleTakeoutSlide() override;

private:
    SolutionDrainer *mDrainer;
    bool cmd_Fill_Solution(int volume);
    bool cmd_Drain_Slot();

    const int MaxVolume;
    const int StepVolume;

    QTimer *m_expiryTimer;
    bool m_isDrainFinished;

    QString m_api;
    QTimer *m_timer;
    enum class Process {
        Idle,

        Fill_Slot,
        WaitF_Fill_Slot_Done,

        WaitF_Change,

        Drain_First,
        Supply_Solution,
        WaitF_Supply_Done,

        Drain_Slot,
        WaitF_Drain_Slot_Done,

        Finish
    } s_state;

    bool isNeedSupply();
    bool isNeedChange();

private slots:
    void onExpiryTimeout_slot();
    void onProcessTimer_slot();
};

#endif // SLOT_FIX_H
