#ifndef SLOT_FIXDRY_H
#define SLOT_FIXDRY_H

#include "slot_base.h"

class SlotFixDry : public SlotBase
{
    Q_OBJECT
public:
    explicit SlotFixDry(int pos, const QString &group, QObject *parent = nullptr);

protected:
    virtual void handlePutinSlide() override;
    virtual void handleTakeoutSlide() override;
};

#endif // SLOT_FIXDRY_H
