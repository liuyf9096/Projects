#include "d_track_1.h"

DTrack1::DTrack1(const QString &dev_id, QObject *parent) : RtDeviceBase(dev_id, parent)
{
    setFunctionMap("Import_Load", (pFunction)&DTrack1::cmd_Import_Load);
    setFunctionMap("Import_Reset", (pFunction)&DTrack1::cmd_Import_Reset);

    setFunctionMap("Exit_E1_Unload", (pFunction)&DTrack1::cmd_Exit_E1_Unload);
    setFunctionMap("Exit_E1_Reset", (pFunction)&DTrack1::cmd_Exit_E1_Reset);

    setFunctionMap("S1_Emergency_Open", (pFunction)&DTrack1::cmd_S1_Emergency_Open);
    setFunctionMap("S1_Emergency_Close", (pFunction)&DTrack1::cmd_S1_Emergency_Close);

    setFunctionMap("S1_Scan_Open", (pFunctionArg1)&DTrack1::cmd_S1_Scan_Open);
    setFunctionMap("S1_Scan_Close", (pFunction)&DTrack1::cmd_S1_Scan_Close);
}

bool DTrack1::cmd_Import_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Import_Reset");
    return true;
}

bool DTrack1::cmd_Import_Load()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Import_Load");
    return true;
}

bool DTrack1::cmd_Exit_E1_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Exit_E1_Reset");
    return true;
}

bool DTrack1::cmd_Exit_E1_Unload()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Exit_E1_Unload");
    return true;
}

bool DTrack1::cmd_S1_Emergency_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S1_Emergency_Close");
    return true;
}

bool DTrack1::cmd_S1_Emergency_Open()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S1_Emergency_Open");
    return true;
}


bool DTrack1::cmd_S1_Scan_Open(int timeout)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {timeout};
    SendComboActionStart("S1_Scan_Open", arg);
    return true;
}

bool DTrack1::cmd_S1_Scan_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S1_Scan_Close");
    return true;
}

