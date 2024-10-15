#include "slotgroup_c1.h"
#include "stain/solution/solution_infuser.h"
#include "stain/solution/solution_drainer.h"
#include "f_common.h"
#include <QDebug>

SlotGroupC1::SlotGroupC1(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{
    mMaxStainCount = FCommon::GetInstance()->getConfigValue("stain", "solution_change", "c1").toInt();
    Q_ASSERT(mMaxStainCount > 0);

    m_water_soaktime_s = FCommon::GetInstance()->getConfigValue("stain", "clean_slots", "water_clean", "soak_time_s").toInt();
    m_alcohol_soaktime_s = FCommon::GetInstance()->getConfigValue("stain", "clean_slots", "alcohol_clean", "soak_time_s").toInt();
    qDebug() << QString("SlotGroup %1, water-soak-time: %2 sec, alcohol-soak-time: %3 sec")
                .arg(group_id).arg(m_water_soaktime_s).arg(m_alcohol_soaktime_s);
}

void SlotGroupC1::slotinit(int pos)
{
    SlotC1 *slot = new SlotC1(pos, mGroupID, this);
    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);
    slot->setMaxStainCount(mMaxStainCount);
    slot->setWaterSoakTime(m_water_soaktime_s);
    slot->setAlcoholSoakTime(m_alcohol_soaktime_s);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    mSlotC1Map.insert(pos, slot);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}

void SlotGroupC1::setSolutionRate(int a, int b)
{
    foreach (auto slot, mSlotC1Map) {
        slot->setSolutionRate(a, b);
    }
}

void SlotGroupC1::setInfuser(SolutionInfuser *infuser)
{
    foreach (auto slot, mSlotC1Map) {
        slot->setInfuser(infuser);
    }
}

void SlotGroupC1::setDrainer(SolutionDrainer *drainer)
{
    foreach (auto slot, mSlotC1Map) {
        slot->setDrainer(drainer);
    }
}

void SlotGroupC1::drainAllSlots(quint64 id)
{
    foreach (auto slot, mSlotC1Map) {
        slot->drainSlot();
    }
    if (id > 0) {
        m_request_id = id;
    }
}

void SlotGroupC1::drainSlots(QVector<int> arr, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotC1Map.contains(pos)) {
            auto slot = mSlotC1Map.value(pos);
            slot->drainSlot();
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupC1::isDrainFinished()
{
    foreach (auto slot, mSlotC1Map) {
        if (slot->isDrainFinished() == false) {
            return false;
        }
    }
    return true;
}

void SlotGroupC1::cleanAllSlots(Detergent d, quint64 id)
{
    foreach (auto slot, mSlotC1Map) {
        slot->cleanSlot(d);
    }
    if (id > 0) {
        m_request_id = id;
    }
}

void SlotGroupC1::cleanSlots(QVector<int> arr, Detergent d, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotC1Map.contains(pos)) {
            auto slot = mSlotC1Map.value(pos);
            slot->cleanSlot(d);
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupC1::isCleanFinished()
{
    foreach (auto slot, mSlotC1Map) {
        if (slot->isCleanFinished() == false) {
            return false;
        }
    }
    return true;
}

