#ifndef SLOT_GROUP_WASH_H
#define SLOT_GROUP_WASH_H

#include "slotgroup_base.h"
#include "slot_wash.h"

class SlotGroupWash : public SlotGroupBase
{
    Q_OBJECT
public:
    explicit SlotGroupWash(const QString &group_id, QObject *parent = nullptr);

    void setWashCount(int count);
    int washCount() { return m_washCount; }

    virtual void drainAllSlots(quint64 id = 0) override;
    virtual void drainSlots(QVector<int> arr, quint64 id = 0) override;
    virtual bool isDrainFinished() override;

protected:
    virtual void slotinit(int pos) override;

private:
    int m_washCount;
    QMap<int, SlotWash*> mSlotWashMap;
};

#endif // SLOT_GROUP_WASH_H
