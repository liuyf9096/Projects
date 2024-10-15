#ifndef SLOT_GROUP_C2_H
#define SLOT_GROUP_C2_H

#include "slotgroup_base.h"
#include "slot_c2.h"

class SolutionInfuser;
class SolutionDrainer;
class SlotGroupC2 : public SlotGroupBase
{
    Q_OBJECT
public:
    explicit SlotGroupC2(const QString &group_id, QObject *parent = nullptr);

    void setSolutionRate(int a, int b);
    void setInfuser(SolutionInfuser *infuser);
    void setDrainer(SolutionDrainer *drainer);

    virtual void drainAllSlots(quint64 id = 0) override;
    virtual void drainSlots(QVector<int> arr, quint64 id = 0) override;
    virtual bool isDrainFinished() override;

    virtual void cleanAllSlots(Detergent d, quint64 id = 0) override;
    virtual void cleanSlots(QVector<int> arr, Detergent d = Detergent_Water, quint64 id = 0) override;
    virtual bool isCleanFinished() override;

protected:
    virtual void slotinit(int pos) override;

private:
    QMap<int, SlotC2*> mSlotC2Map;
};

#endif // SLOT_GROUP_C2_H
