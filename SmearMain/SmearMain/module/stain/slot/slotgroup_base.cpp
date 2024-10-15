#include "slotgroup_base.h"
#include "device/rt_device_manager.h"
#include "file/f_file_manager.h"

#include <QDebug>

SlotGroupBase::SlotGroupBase(const QString &group_id, QObject *parent)
    : QObject{parent}
    , mGroupID(group_id)
    , m_request_id(0)
{
    mMaxStainCount = 1;
    mDuration = 0;
    m_water_soaktime_s = 0;
    m_alcohol_soaktime_s = 0;
}

void SlotGroupBase::setSlot(int from, int to)
{
    if (!mSlotList.isEmpty()) {
        qDebug() << "!!!!!!!!!!????";
        qDeleteAll(mSlotList);
    }

    if (from <= to) {
        for (int i = from; i <= to; ++i) {
            slotinit(i);
        }
    } else {
        for (int i = from; i >= to; --i) {
            slotinit(i);
        }
    }
}

void SlotGroupBase::prepareSlotPos(int pos, const QString &sid)
{
    if (mSlotMap.contains(pos)) {
        SlotBase *slot = mSlotMap.value(pos);
        slot->prepareSolution(sid);
    }
}

void SlotGroupBase::slotinit(int pos)
{
    SlotBase *slot = new SlotBase(pos, mGroupID, this);
    mSlotList.append(slot);
    mSlotMap.insert(pos, slot);
    connect(slot, &SlotBase::onGripperRequest_signal,
            this, &SlotGroupBase::onGripperRequest_signal);

    slot->setDeviceManager(RtDeviceManager::GetInstance());
    slot->setLogLabel(mGroupID);

    qDebug() << mGroupID << "slot:" << slot->mPos;
}

int SlotGroupBase::getVacantSlot()
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        if (slot->isAvailable()) {
            return slot->mPos;
        }
    }
    return -1;
}

void SlotGroupBase::setDuration(int sec)
{
    mDuration = sec;

    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        slot->setDuration(sec);
    }
    qDebug().noquote() << QString("%1 set duration: %2s").arg(mGroupID).arg(sec);
}

void SlotGroupBase::stainStart(const QString &sid, int pos)
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        if (slot->mPos == pos) {
            slot->putinSlide(sid);
        }
    }
}

void SlotGroupBase::takeOutSample(const QString &sid, int pos)
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        if (slot->mPos == pos) {
            slot->takeoutSlide(sid);
        }
    }
}

void SlotGroupBase::setStainMethod(const QString &method)
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        slot->setStainMethod(method);
    }
}

void SlotGroupBase::setRemainSlideSlot(int pos, const QString &slide_id)
{
    if (mSlotMap.contains(pos)) {
        SlotBase *slot = mSlotMap.value(pos);
        slot->setRemainSlideSlot(slide_id);
    }
}

void SlotGroupBase::setMaxStainCount(int stainCount)
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        slot->setMaxStainCount(stainCount);
    }
}

void SlotGroupBase::setSolutionExpiryTime(int min)
{
    for (int i = 0; i < mSlotList.count(); ++i) {
        SlotBase *slot = mSlotList.at(i);
        slot->setSolutionExpiryTime(min);
    }
}

SlotBase *SlotGroupBase::slotAt(int pos)
{
    if (mSlotMap.contains(pos)) {
        SlotBase *slot = mSlotMap.value(pos);
        return slot;
    }
    return nullptr;
}

