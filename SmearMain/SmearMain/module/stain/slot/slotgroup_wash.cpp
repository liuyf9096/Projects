#include "slotgroup_wash.h"

#include "slot_wash.h"
#include <QDebug>

SlotGroupWash::SlotGroupWash(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{
    m_washCount = 0;
}

void SlotGroupWash::slotinit(int pos)
{
    SlotWash *slot = new SlotWash(pos, mGroupID, this);
    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    mSlotWashMap.insert(pos, slot);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}

void SlotGroupWash::setWashCount(int count)
{
    m_washCount = count;
    qDebug() << "wash slot set wash count:" << count;

    foreach (auto slot, mSlotWashMap) {
        slot->setWashCount(count);
    }
}

void SlotGroupWash::drainAllSlots(quint64 id)
{
    foreach (auto slot, mSlotWashMap) {
        slot->drainSlot();
    }

    if (id > 0) {
        m_request_id = id;
    }
}

void SlotGroupWash::drainSlots(QVector<int> arr, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotWashMap.contains(pos)) {
            auto slot = mSlotWashMap.value(pos);
            slot->drainSlot();
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupWash::isDrainFinished()
{
    foreach (auto slot, mSlotWashMap) {
        if (slot->isDrainFinished() == false) {
            return false;
        }
    }
    return true;
}

