#ifndef SLOT_GROUP_FIX_H
#define SLOT_GROUP_FIX_H

#include "slotgroup_base.h"
#include "slot_fix.h"

class SolutionDrainer;
class SlotGroupFix : public SlotGroupBase
{
    Q_OBJECT
public:
    explicit SlotGroupFix(const QString &group_id, QObject *parent = nullptr);

    void setDrainer(SolutionDrainer *drainer);
    void handleSlotRequest(const JPacket &p);

    virtual void drainSlots(QVector<int> arr, quint64 id = 0) override;
    virtual bool isDrainFinished() override;

protected:
    virtual void slotinit(int pos) override;

private:
    QMap<int, SlotFix*> mSlotFixMap;
};

#endif // SLOT_GROUP_FIX_H
