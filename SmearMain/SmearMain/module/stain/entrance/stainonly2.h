#ifndef STAINONLY2_H
#define STAINONLY2_H

#include "stainonly_base.h"

class StainOnly2 : public StainOnlyBase
{
    Q_OBJECT
public:
    explicit StainOnly2(const QString &mid, int from, int to, QObject *parent = nullptr);

protected:
    virtual void setLightRun() override;
    virtual void setLightStop() override;
    virtual void setLightError() override;
    virtual void setLightAlarm() override;
    virtual void clearLightAlarm() override;
};

#endif // STAINONLY2_H
