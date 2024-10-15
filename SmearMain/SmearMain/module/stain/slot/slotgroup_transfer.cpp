#include "slotgroup_transfer.h"

#include "slot_transfer.h"
#include <QDebug>

SlotGroupTransfer::SlotGroupTransfer(const QString &group_id, QObject *parent)
    : SlotGroupBase{group_id, parent}
{

}

void SlotGroupTransfer::slotinit(int pos)
{
    SlotTransfer *slot = new SlotTransfer(pos, mGroupID, this);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}
