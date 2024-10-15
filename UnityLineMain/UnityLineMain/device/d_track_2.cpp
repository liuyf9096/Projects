#include "d_track_2.h"

DTrack2::DTrack2(const QString &dev_id, QObject *parent) : RtDeviceBase(dev_id, parent)
{
    setFunctionMap("S3_Emergency_Close", (pFunction)&DTrack2::cmd_S3_Emergency_Close);
    setFunctionMap("S3_Emergency_Open", (pFunction)&DTrack2::cmd_S3_Emergency_Open);
    setFunctionMap("S3_Scan_Open", (pFunctionArg1)&DTrack2::cmd_S3_Scan_Open);
    setFunctionMap("S3_Scan_Close", (pFunction)&DTrack2::cmd_S3_Scan_Close);

    setFunctionMap("Rotate_Start", (pFunction)&DTrack2::cmd_Rotate_Start);
}

bool DTrack2::cmd_S3_Emergency_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S3_Emergency_Close");
    return true;
}

bool DTrack2::cmd_S3_Emergency_Open()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S3_Emergency_Open");
    return true;
}

bool DTrack2::cmd_S3_Scan_Open(int timeout)
{
    QJsonArray arg = {timeout};
    SendComboActionStart("S3_Scan_Open", arg);
    return true;
}

bool DTrack2::cmd_S3_Scan_Close()
{
    SendComboActionStart("S3_Scan_Close");
    return true;
}

bool DTrack2::cmd_Rotate_Start()
{
    SendComboActionStart("Rotate_Start");
    return true;
}

