#ifndef STAINONLY1_H
#define STAINONLY1_H

#include "stainonly_base.h"

class StainOnly1 : public StainOnlyBase
{
    Q_OBJECT
public:
    explicit StainOnly1(const QString &mid, int from, int to, QObject *parent = nullptr);

protected:
    virtual void setLightRun() override;
    virtual void setLightStop() override;
    virtual void setLightError() override;
    virtual void setLightAlarm() override;
    virtual void clearLightAlarm() override;
};

#endif // STAINONLY1_H
