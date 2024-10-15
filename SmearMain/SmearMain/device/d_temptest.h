#ifndef D_TEMPTEST_H
#define D_TEMPTEST_H

#include "rt_device_base.h"

class DTempTest : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DTempTest(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DTempTest() {}

    virtual bool cmd_Reset() override;

    bool cmd_QueryTemperatureWet();
    bool cmd_QueryPressurePN();
};

#endif // D_TEMPTEST_H
