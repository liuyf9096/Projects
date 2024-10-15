#ifndef SLOT_WASH_H
#define SLOT_WASH_H

#include "slot_base.h"

class SlotWash : public SlotBase
{
    Q_OBJECT
public:
    explicit SlotWash(int pos, const QString &group, QObject *parent = nullptr);

    void setWashCount(int sec);

    bool drainSlot(quint64 id = 0);
    bool isDrainFinished() { return m_isDrainFinished; }
    bool isCleanFinished() { return m_isDrainFinished; }

    virtual void prepareSolution(const QString &sid) override;

protected:
    virtual void handlePutinSlide() override;
    virtual void handleTakeoutSlide() override;
    virtual void onDurationTimeout() override;

private:
    int m_washCountMax;
    int m_washCount;

    bool cmd_Slot_Wash();
    bool cmd_Slot_Fill();
    bool cmd_Slot_Drain();
    bool isSlotDrainFinished();

    QString m_api;
    QTimer *m_washTimer;
    enum class Process {
        Idle,

        Fill_Slot,
        WaitF_Fill_Slot_Done,

        WaitF_Slot_In,

        Drain_Slot,
        WaitF_Drain_Slot_Done,

        Send_Wash_Cmd,
        WaitF_Wash_Done,

        Finish
    } s_state;

    QTimer *m_drainTimer;
    bool m_isDrainFinished;
    enum class DrainProcess {
        Idle,

        Drain_Slot,
        WaitF_Drain_Slot_Done,

        Finish
    } s_drain;

private slots:
    void onProcessTimer_slot();
    void onDrainProcess_slot();
};

#endif // SLOT_WASH_H
