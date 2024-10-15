#ifndef SLOT_GROUP_FIXDRY_H
#define SLOT_GROUP_FIXDRY_H

#include "slotgroup_base.h"

class SlotGroupFixDry : public SlotGroupBase
{
    Q_OBJECT
public:
    explicit SlotGroupFixDry(const QString &group_id, QObject *parent = nullptr);

protected:
    virtual void slotinit(int pos) override;
};

#endif // SLOT_GROUP_FIXDRY_H
