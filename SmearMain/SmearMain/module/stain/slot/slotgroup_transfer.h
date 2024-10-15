#ifndef SLOT_GROUP_TRANSFER_H
#define SLOT_GROUP_TRANSFER_H

#include "slotgroup_base.h"

class SlotGroupTransfer : public SlotGroupBase
{
    Q_OBJECT
public:
    explicit SlotGroupTransfer(const QString &group_id, QObject *parent = nullptr);

protected:
    virtual void slotinit(int pos) override;
};

#endif // SLOT_GROUP_TRANSFER_H
