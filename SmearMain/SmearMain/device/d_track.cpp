#include "d_track.h"

DTrack::DTrack(const QString &dev_id, QObject *parent)
    : RtDeviceBase(dev_id, parent)
{
    m_Load_Lock = false;

    setFunctionMap("C1_Reset", (pFunction)&DTrack::cmd_C1_Reset);
    setFunctionMap("C1_Pos_Import", (pFunction)&DTrack::cmd_C1_Pos_Import);
    setFunctionMap("C1_Pos_Scan", (pFunctionArg1)&DTrack::cmd_C1_Pos_Scan);
    setFunctionMap("C1_Pos_Test", (pFunctionArg1)&DTrack::cmd_C1_Pos_Test);
    setFunctionMap("C1_Pos_Export", (pFunction)&DTrack::cmd_C1_Pos_Export);
    setFunctionMap("C1_Pos_Left", (pFunctionArg1)&DTrack::cmd_C1_Pos_Left);
    setFunctionMap("C1_Pos_Right", (pFunctionArg1)&DTrack::cmd_C1_Pos_Right);

    setFunctionMap("C2_Reset", (pFunction)&DTrack::cmd_C2_Reset);
    setFunctionMap("C2_Pos_Import", (pFunction)&DTrack::cmd_C2_Pos_Import);
    setFunctionMap("C2_Pos_Scan", (pFunctionArg1)&DTrack::cmd_C2_Pos_Scan);
    setFunctionMap("C2_Pos_Test", (pFunctionArg1)&DTrack::cmd_C2_Pos_Test);
    setFunctionMap("C2_Pos_Export", (pFunction)&DTrack::cmd_C2_Pos_Export);
    setFunctionMap("C2_Pos_Left", (pFunctionArg1)&DTrack::cmd_C2_Pos_Left);
    setFunctionMap("C2_Pos_Right", (pFunctionArg1)&DTrack::cmd_C2_Pos_Right);

    setFunctionMap("Import_Reset", (pFunction)&DTrack::cmd_Import_Reset);
    setFunctionMap("Import_Load", (pFunction)&DTrack::cmd_Import_Load);

    setFunctionMap("Export_Reset", (pFunction)&DTrack::cmd_Export_Reset);
    setFunctionMap("Export_Unload", (pFunction)&DTrack::cmd_Export_Unload);

    setFunctionMap("Emergency_Close", (pFunction)&DTrack::cmd_Emergency_Close);
    setFunctionMap("Emergency_Open", (pFunction)&DTrack::cmd_Emergency_Open);

    setFunctionMap("Scan_Open", (pFunctionArg1)&DTrack::cmd_Scan_Open);
    setFunctionMap("Scan_Close", (pFunction)&DTrack::cmd_Scan_Close);
}

bool DTrack::cmd_C1_Reset()
{
    if (isResetOk() == false) {
        return false;
    }

    m_Lock_Reset = true;
    SendComboActionStart("C1_Reset");
    return true;
}

bool DTrack::cmd_C1_Pos_Import()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C1_Pos_Import");
    return true;
}

bool DTrack::cmd_C1_Pos_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Scan", arg);
    return true;
}

bool DTrack::cmd_C1_Pos_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Test", arg);
    return true;
}

bool DTrack::cmd_C1_Pos_Export()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C1_Pos_Export");
    return true;
}

bool DTrack::cmd_C1_Pos_Left(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Left", arg);
    return true;
}

bool DTrack::cmd_C1_Pos_Right(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Right", arg);
    return true;
}

bool DTrack::cmd_C2_Reset()
{
    if (isResetOk() == false) {
        return false;
    }

    m_Lock_Reset = true;
    SendComboActionStart("C2_Reset");
    return true;
}

bool DTrack::cmd_C2_Pos_Import()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C2_Pos_Import");
    return true;
}

bool DTrack::cmd_C2_Pos_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Scan", arg);
    return true;
}

bool DTrack::cmd_C2_Pos_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Test", arg);
    return true;
}

bool DTrack::cmd_C2_Pos_Export()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C2_Pos_Export");
    return true;
}

bool DTrack::cmd_C2_Pos_Left(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Left", arg);
    return true;
}

bool DTrack::cmd_C2_Pos_Right(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Right", arg);
    return true;
}

bool DTrack::cmd_Import_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Import_Reset");
    return true;
}

bool DTrack::cmd_Import_Load()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Import_Load");
    return true;
}

bool DTrack::cmd_Export_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Export_Reset");
    return true;
}

bool DTrack::cmd_Export_Unload()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Export_Unload");
    return true;
}

bool DTrack::cmd_Emergency_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Emergency_Close");
    return true;
}

bool DTrack::cmd_Emergency_Open()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Emergency_Open");
    return true;
}

bool DTrack::cmd_Scan_Open(int timeout)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {timeout};
    SendComboActionStart("Scan_Open", arg);
    return true;
}

bool DTrack::cmd_Scan_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Scan_Close");
    return true;
}

bool DTrack::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (api == "C1_Reset" || api == "C2_Reset") {
        m_Lock_Reset = false;
    }
    return RtDeviceBase::handleReceiveResult(api, resValue);
}

void DTrack::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (api == "C1_Reset" || api == "C2_Reset") {
        m_Lock_Reset = false;
    }
    return RtDeviceBase::handleReceiveResultError(api, errorObj);
}


