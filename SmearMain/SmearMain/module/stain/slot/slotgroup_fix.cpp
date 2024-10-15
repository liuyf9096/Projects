#include "slotgroup_fix.h"
#include "f_common.h"
#include <QDebug>

SlotGroupFix::SlotGroupFix(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{
    mMaxStainCount = FCommon::GetInstance()->getConfigValue("stain", "solution_change", "fix").toInt();
    Q_ASSERT(mMaxStainCount > 0);
}

void SlotGroupFix::setDrainer(SolutionDrainer *drainer)
{
    foreach (auto slot, mSlotFixMap) {
        slot->setDrainer(drainer);
    }
}

void SlotGroupFix::slotinit(int pos)
{
    SlotFix *slot = new SlotFix(pos, mGroupID, this);
    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);
    slot->setMaxStainCount(mMaxStainCount);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    mSlotFixMap.insert(pos, slot);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}

void SlotGroupFix::handleSlotRequest(const JPacket &p)
{
    SlotFix *slot = nullptr;
    if (p.api == "DrainFix1") {
        slot = qobject_cast<SlotFix *>(mSlotList.at(0));
    } else if (p.api == "DrainFix2") {
        slot = qobject_cast<SlotFix *>(mSlotList.at(1));
    }

    if (slot) {
        slot->drainSlot(p.id);
    }
}

void SlotGroupFix::drainSlots(QVector<int> arr, quint64 id)
{
    foreach (auto pos, arr) {
        if (mSlotFixMap.contains(pos)) {
            auto slot = mSlotFixMap.value(pos);
            slot->drainSlot();
        }
    }
    if (id > 0) {
        m_request_id = id;
    }
}

bool SlotGroupFix::isDrainFinished()
{
    foreach (auto slot, mSlotFixMap) {
        if (slot->isDrainFinished() == false) {
            return false;
        }
    }
    return true;
}
