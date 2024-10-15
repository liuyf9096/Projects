#include "d_temptest.h"

DTempTest::DTempTest(const QString &dev_id, QObject *parent) : RtDeviceBase(dev_id, parent)
{
    setFunctionMap("QueryTemperatureWet", (pFunction)&DTempTest::cmd_QueryTemperatureWet);
    setFunctionMap("QueryPressurePN", (pFunction)&DTempTest::cmd_QueryPressurePN);
}

bool DTempTest::cmd_Reset()
{
    return true;
}

bool DTempTest::cmd_QueryTemperatureWet()
{
    SendCommand("QueryTemperatureWet");
    return true;
}

bool DTempTest::cmd_QueryPressurePN()
{
    SendCommand("QueryPressurePN");
    return true;
}
