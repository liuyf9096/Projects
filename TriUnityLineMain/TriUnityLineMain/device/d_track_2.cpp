#include "d_track_2.h"

DTrack2::DTrack2(const QString &dev_id, QObject *parent) : RtDeviceBase(dev_id, parent)
{
    setFunctionMap("C1_Reset", (pFunction)&DTrack2::cmd_C1_Reset);
    setFunctionMap("C1_Pos_Import", (pFunction)&DTrack2::cmd_C1_Pos_Import);
    setFunctionMap("C1_Pos_S1_Scan", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S1_Scan);
    setFunctionMap("C1_Pos_S1_Test", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S1_Test);
    setFunctionMap("C1_Pos_S2_Scan", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S2_Scan);
    setFunctionMap("C1_Pos_S2_Test", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S2_Test);
    setFunctionMap("C1_Pos_S3_Scan", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S3_Scan);
    setFunctionMap("C1_Pos_S3_Test", (pFunctionArg1)&DTrack2::cmd_C1_Pos_S3_Test);
    setFunctionMap("C1_Pos_Exit_E1", (pFunction)&DTrack2::cmd_C1_Pos_Exit_E1);
    setFunctionMap("C1_Pos_Exit_E2", (pFunction)&DTrack2::cmd_C1_Pos_Exit_E2);
    setFunctionMap("C1_Pos_Left", (pFunctionArg1)&DTrack2::cmd_C1_Pos_Left);
    setFunctionMap("C1_Pos_Right", (pFunctionArg1)&DTrack2::cmd_C1_Pos_Right);

    setFunctionMap("C2_Reset", (pFunction)&DTrack2::cmd_C2_Reset);
    setFunctionMap("C2_Pos_Import", (pFunction)&DTrack2::cmd_C2_Pos_Import);
    setFunctionMap("C2_Pos_S1_Scan", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S1_Scan);
    setFunctionMap("C2_Pos_S1_Test", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S1_Test);
    setFunctionMap("C2_Pos_S2_Scan", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S2_Scan);
    setFunctionMap("C2_Pos_S2_Test", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S2_Test);
    setFunctionMap("C2_Pos_S3_Scan", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S3_Scan);
    setFunctionMap("C2_Pos_S3_Test", (pFunctionArg1)&DTrack2::cmd_C2_Pos_S3_Test);
    setFunctionMap("C2_Pos_Exit_E1", (pFunction)&DTrack2::cmd_C2_Pos_Exit_E1);
    setFunctionMap("C2_Pos_Exit_E2", (pFunction)&DTrack2::cmd_C2_Pos_Exit_E2);
    setFunctionMap("C2_Pos_Left", (pFunctionArg1)&DTrack2::cmd_C2_Pos_Left);
    setFunctionMap("C2_Pos_Right", (pFunctionArg1)&DTrack2::cmd_C2_Pos_Right);

    setFunctionMap("Exit_E2_Unload", (pFunction)&DTrack2::cmd_Exit_E2_Unload);
    setFunctionMap("Exit_E2_Reset", (pFunction)&DTrack2::cmd_Exit_E2_Reset);

    setFunctionMap("S2_Emergency_Open", (pFunction)&DTrack2::cmd_S2_Emergency_Open);
    setFunctionMap("S2_Emergency_Close", (pFunction)&DTrack2::cmd_S2_Emergency_Close);

    setFunctionMap("S3_Emergency_Close", (pFunction)&DTrack2::cmd_S3_Emergency_Close);
    setFunctionMap("S3_Emergency_Open", (pFunction)&DTrack2::cmd_S3_Emergency_Open);

    setFunctionMap("S2_Scan_Open", (pFunctionArg1)&DTrack2::cmd_S2_Scan_Open);
    setFunctionMap("S2_Scan_Close", (pFunction)&DTrack2::cmd_S2_Scan_Close);

    setFunctionMap("S3_Scan_Open", (pFunctionArg1)&DTrack2::cmd_S3_Scan_Open);
    setFunctionMap("S3_Scan_Close", (pFunction)&DTrack2::cmd_S3_Scan_Close);
}

bool DTrack2::cmd_C1_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Lock_1 == false) {
        m_Lock_1 = true;
        SendComboActionStart("C1_Reset");
        return true;
    }
    return false;
}

bool DTrack2::cmd_C1_Pos_Import()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C1_Pos_Import");
    return true;
}

bool DTrack2::cmd_C1_Pos_S1_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S1_Scan", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_S1_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S1_Test", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_S2_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S2_Scan", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_S2_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S2_Test", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_S3_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S3_Scan", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_S3_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_S3_Test", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_Exit_E1()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C1_Pos_Exit_E1");
    return true;
}

bool DTrack2::cmd_C1_Pos_Exit_E2()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C1_Pos_Exit_E2");
    return true;
}

bool DTrack2::cmd_C1_Pos_Left(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Left", arg);
    return true;
}

bool DTrack2::cmd_C1_Pos_Right(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C1_Pos_Right", arg);
    return true;
}

bool DTrack2::cmd_C2_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Lock_1 == false) {
        m_Lock_1 = true;
        SendComboActionStart("C2_Reset");
        return true;
    }
    return false;
}

bool DTrack2::cmd_C2_Pos_Import()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C2_Pos_Import");
    return true;
}

bool DTrack2::cmd_C2_Pos_S1_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S1_Scan", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_S1_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S1_Test", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_S2_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S2_Scan", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_S2_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S2_Test", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_S3_Scan(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S3_Scan", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_S3_Test(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_S3_Test", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_Exit_E1()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C2_Pos_Exit_E1");
    return true;
}

bool DTrack2::cmd_C2_Pos_Exit_E2()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("C2_Pos_Exit_E2");
    return true;
}

bool DTrack2::cmd_C2_Pos_Left(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Left", arg);
    return true;
}

bool DTrack2::cmd_C2_Pos_Right(int pos)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {pos};
    SendComboActionStart("C2_Pos_Right", arg);
    return true;
}

bool DTrack2::cmd_Exit_E2_Unload()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Exit_E2_Unload");
    return true;
}

bool DTrack2::cmd_Exit_E2_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Exit_E2_Reset");
    return true;
}

bool DTrack2::cmd_S2_Emergency_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S2_Emergency_Close");
    return true;
}

bool DTrack2::cmd_S2_Emergency_Open()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S2_Emergency_Open");
    return true;
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

bool DTrack2::cmd_S2_Scan_Open(int timeout)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {timeout};
    SendComboActionStart("S2_Scan_Open", arg);
    return true;
}

bool DTrack2::cmd_S2_Scan_Close()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("S2_Scan_Close");
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

bool DTrack2::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (api == "C1_Reset" || api == "C2_Reset") {
        m_Lock_1 = false;
    }
    return RtDeviceBase::handleReceiveResult(api, resValue);
}

void DTrack2::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (api == "C1_Reset" || api == "C2_Reset") {
        m_Lock_1 = false;
    }
    return RtDeviceBase::handleReceiveResultError(api, errorObj);
}
