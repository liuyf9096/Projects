#include "slotgroup_c2.h"
#include "stain/solution/solution_infuser.h"
#include "stain/solution/solution_drainer.h"
#include "f_common.h"
#include <QDebug>

SlotGroupC2::SlotGroupC2(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{
    mMaxStainCount = FCommon::GetInstance()->getConfigValue("stain", "solution_change", "c2").toInt();
    Q_ASSERT(mMaxStainCount > 0);

    m_water_soaktime_s = FCommon::GetInstance()->getConfigValue("stain", "clean_slots", "water_clean", "soak_time_s").toInt();
    m_alcohol_soaktime_s = FCommon::GetInstance()->getConfigValue("stain", "clean_slots", "alcohol_clean", "soak_time_s").toInt();
    qDebug() << QString("SlotGroup %1, water-soak-time: %2 sec, alcohol-soak-time: %3 sec")
                .arg(group_id).arg(m_water_soaktime_s).arg(m_alcohol_soaktime_s);
}

void SlotGroupC2::slotinit(int pos)
{
    SlotC2 *slot = new SlotC2(pos, mGroupID, this);
    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);
    slot->setMaxStainCount(mMaxStainCount);
    slot->setWaterSoakTime(m_water_soaktime_s);
    slot->setAlcoholSoakTime(m_alcohol_soaktime_s);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    mSlotC2Map.insert(pos, slot);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}

void SlotGroupC2::setSolutionRate(int a, int b)
{
    foreach (auto slot, mSlotC2Map) {
        slot->setSolutionRate(a, b);
    }
}

void SlotGroupC2::setInfuser(SolutionInfuser *infuser)
{
    foreach (auto slot, mSlotC2Map) {
        slot->setInfuser(infuser);
    }
}

void SlotGroupC2::setDrainer(SolutionDrainer *drainer)
{
    foreach (auto slot, mSlotC2Map) {
        slot->setDrainer(drainer);
    }
}

void SlotGroupC2::drainAllSlots(quint64 id)
{
    foreach (auto slot, mSlotC2Map) {
        slot->drainSlot();
    }

    if (id > 0) {
        m_request_id = id;
    }
}

void SlotGroupC2::drainSlots(QVector<int> arr, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotC2Map.contains(pos)) {
            auto slot = mSlotC2Map.value(pos);
            slot->drainSlot();
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupC2::isDrainFinished()
{
    foreach (auto slot, mSlotC2Map) {
        if (slot->isDrainFinished() == false) {
            return false;
        }
    }
    return true;
}

void SlotGroupC2::cleanAllSlots(Detergent d, quint64 id)
{
    foreach (auto slot, mSlotC2Map) {
        slot->cleanSlot(d);
    }
    if (id > 0) {
        m_request_id = id;
    }
}

void SlotGroupC2::cleanSlots(QVector<int> arr, Detergent d, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotC2Map.contains(pos)) {
            auto slot = mSlotC2Map.value(pos);
            slot->cleanSlot(d);
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupC2::isCleanFinished()
{
    foreach (auto slot, mSlotC2Map) {
        if (slot->isCleanFinished() == false) {
            return false;
        }
    }
    return true;
}
