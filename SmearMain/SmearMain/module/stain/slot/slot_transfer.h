#ifndef SLOT_TRANSFER_H
#define SLOT_TRANSFER_H

#include "slot_base.h"

class SlotTransfer : public SlotBase
{
    Q_OBJECT
public:
    explicit SlotTransfer(int pos, const QString &group, QObject *parent = nullptr);

protected:
    virtual void handlePutinSlide() override;
    virtual void handleTakeoutSlide() override;
};

#endif // SLOT_TRANSFER_H
