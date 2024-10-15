#include "slotgroup_fixdry.h"

#include "slot_fixdry.h"
#include <QDebug>

SlotGroupFixDry::SlotGroupFixDry(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{

}

void SlotGroupFixDry::slotinit(int pos)
{
    SlotFixDry *slot = new SlotFixDry(pos, mGroupID, this);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}
