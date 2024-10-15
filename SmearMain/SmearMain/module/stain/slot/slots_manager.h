#ifndef SLOTS_MANAGER_H
#define SLOTS_MANAGER_H

#include "m_manager_base.h"
#include "sample/rt_sample_manager.h"

#include "slotgroup_base.h"
#include "slotgroup_fix.h"
#include "slotgroup_fixdry.h"
#include "slotgroup_c1.h"
#include "slotgroup_c2.h"
#include "slotgroup_wash.h"
#include "slotgroup_transfer.h"
#include "stain/solution/solution_drainer.h"
#include "stain/solution/solution_infuser.h"

#include <QMap>

class SlotsManager : public MManagerBase
{
    Q_OBJECT
public:
    static SlotsManager *GetInstance();

    void _setSlotDefaultDuration(int cycle_sec);
    void setSlotParams(const QJsonArray &arr);
    void setStainMethod(const QString &method);
    bool containGroup(const QString &group);

    void handleSlotRequest(const JPacket &p);
    void handleRemainSlides(const QJsonArray &arr);

    void startFillPool(quint64 id, const QJsonObject &obj);

    void updateRemainingTime(const QString &sid);
    int getStainTimeRemaining(QSharedPointer<RtSlide> slide);

    void slideStainStart(const QString &group_id, const QString &sid, int pos = -1);
    void takingOutSample(const QString &group_id, const QString &sid, int pos = -1);
    void takeOutSample(const QString &group_id, const QString &sid, int pos = -1);

    int applySlotPos(const QString &group_id);
    void prepareSlotPos(int pos, const QString &sid);

    void setSolutionExpiration(const QJsonArray &arr);

    void drainAllSlots(quint64 id = 0);
    void drainSlots(QVector<int> arr, quint64 id = 0);
//    void drainSlots(QJsonArray arr, quint64 id = 0);
    QVector<int> m_drain_slot_arr;

    void cleanAllSlots(JPacket p);
    void cleanAllSlots(Detergent d, quint64 id);
    void cleanSlots(QVector<int> arr, Detergent d = Detergent_Water, quint64 id = 0);
    void cleanSlots(QJsonArray arr, Detergent d = Detergent_Water, quint64 id = 0);
    void cleanSlots(QJsonArray arr, const QString &detergent, quint64 id = 0);
    void cleanRemainSlots(quint64 id = 0);
    bool isCleanSlotsFinished() { return m_isCleanSlotsFinished; }

    SlotBase *slotAt(int pos);

    /* Components */
    SlotGroupFix *mGroupFix;
    SlotGroupFixDry *mGroupFixDry;
    SlotGroupC1 *mGroupC1;
    SlotGroupC2 *mGroupC2;
    SlotGroupWash *mGroupWash;
    SlotGroupTransfer *mGroupTransfer;

signals:
    void onCheckNewSample_signal();
    void onStartStain_signal(int pos, const QString &sid, int interval);
    void onTakeOutOfPool_signal(int pos, const QString &sid);

private:
    explicit SlotsManager(QObject *parent = nullptr);

    void _setSlots();

    QMap<QString, SlotGroupBase *> mSlotGroupMap;

    bool m_cleanAllSlots;
    QVector<int> m_cleanFixArr;
    QVector<int> m_cleanC1Arr;
    QVector<int> m_cleanC2Arr;
    bool m_drainWash;

    quint64 m_drainSlots_id;
    QTimer *m_drainSlotsTimer;
    bool m_isDrainSlotsFinished;
    enum class DrainSlotsState {
        Idle,

        Drain_Slot_C1,
        WaitF_Drain_Slot_C1_Done,

        Drain_Slot_C2,
        WaitF_Drain_Slot_C2_Done,

        Finish
    } s_drainSlots;

    Detergent m_detergent;
    quint64 m_cleanSlots_id;
    QTimer *m_cleanSlotsTimer;
    bool m_isCleanSlotsFinished;
    enum class CleanSlotsState {
        Idle,

        Drain_Fix,
        WaitF_Drain_Fix_Done,

        Clean_Slot_C1,
        WaitF_Clean_Slot_C1_Done,

        Clean_Slot_C2,
        WaitF_Clean_Slot_C2_Done,

        Drain_Wash,
        WaitF_Drain_Wash_Done,

        Finish
    } s_cleanSlots;

private slots:
    void drainAllSlots_slot();
    void cleanAllSlots_slot();
};

#endif // SLOTS_MANAGER_H
