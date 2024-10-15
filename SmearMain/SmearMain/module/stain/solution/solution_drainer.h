#ifndef SOLUTION_DRAINER_H
#define SOLUTION_DRAINER_H

#include "module_base.h"

class SolutionDrainer : public DModuleBase
{
    Q_OBJECT
public:
    explicit SolutionDrainer(const QString &mid, QObject *parent = nullptr);

    void addRequest(int pos);
    void addRequest(QList<int> posList);
    bool isIdle() { return m_requestList.isEmpty(); }
    void addDrainCount(int count);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();
    int MaxDrainCount;

    int m_drainCount;
    QTimer *mDrainSlotTimer;
    QJsonObject m_drainSlotTaskObj;
    QString m_api;
    int m_handlingPos;

    void drainSlotProcess();

    enum class DrainSlot {
        Idle,
        Send_Drain_Cmd,
        WaitF_Drain_Cmd_Done
    } s_drainslot;

    QList<int> m_requestList;

    void drainTankProcess();
    bool m_isCleanTank;

    int m_floaterAlarmValue;
    bool m_checkFloaterAlarm;

    QTimer *m_fixDrainTimer;
    enum class DrainWaste {
        Idle,
        Send_Drain_Tank_Cmd,
        Reset_Drain_Needle,
        WaitF_Drain_Tank_Done
    } s_drainwaste;

private slots:
    void onDrainSlotTimer_slot();
    void onDrainWasteTankTimer_slot();
};

#endif // SOLUTION_DRAINER_H
