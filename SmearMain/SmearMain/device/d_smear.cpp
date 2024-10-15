#include "d_smear.h"

DSmear::DSmear(const QString &dev_id, QObject *parent)
    : RtDeviceBase(dev_id, parent)
{
    m_SmearCart_Lock = false;

    SmearCart_List << "NewSlide1_ToAddSamplingPos" << "NewSlide2_ToAddSamplingPos"
                   << "NewSlide1_ToPrintPos" << "NewSlide2_ToPrintPos"
                   << "PrintSlideInfo"
                   << "Slide_FrPrintToAddSamplePos" << "Slide_FrPrintToStainCartPos"
                   << "SmearCart_Reset";

    setFunctionMap("CleanLiquidSystem", (pFunction)&DSmear::cmd_CleanLiquidSystem);

    setFunctionMap("NewSlide1_ToAddSamplingPos", (pFunction)&DSmear::cmd_NewSlide1_ToAddSamplingPos);
    setFunctionMap("NewSlide2_ToAddSamplingPos", (pFunction)&DSmear::cmd_NewSlide2_ToAddSamplingPos);
    setFunctionMap("NewSlide1_ToPrintPos", (pFunction)&DSmear::cmd_NewSlide1_ToPrintPos);
    setFunctionMap("NewSlide2_ToPrintPos", (pFunction)&DSmear::cmd_NewSlide2_ToPrintPos);

    setFunctionMap("PrintSlideInfo", (pFunction)&DSmear::cmd_PrintSlideInfo);
    setFunctionMap("Slide_FrPrintToAddSamplePos", (pFunction)&DSmear::cmd_Slide_FrPrintToAddSamplePos);
    setFunctionMap("Slide_FrPrintToStainCartPos", (pFunction)&DSmear::cmd_Slide_FrPrintToStainCartPos);
    setFunctionMap("SmearCart_Reset", (pFunction)&DSmear::cmd_SmearCart_Reset);

    setFunctionMap("Smear", (pFunctionArg10)&DSmear::cmd_Smear);
    setFunctionMap("Smear2", (pFunctionArg5)&DSmear::cmd_Smear2);

    setFunctionMap("FillWashPool", (pFunction)&DSmear::cmd_FillWashPool);
    setFunctionMap("CleanSmearBlade", (pFunction)&DSmear::cmd_CleanSmearBlade);
    setFunctionMap("CleanSmearBlade_Aging", (pFunction)&DSmear::cmd_CleanSmearBlade_Aging);
    setFunctionMap("CleanSmearBlade_Maintian", (pFunction)&DSmear::cmd_CleanSmearBlade_Maintian);

    setFunctionMap("WashPoolDrain_Open", (pFunction)&DSmear::cmd_WashPoolDrain_Open);
    setFunctionMap("WashPoolDrain_Close", (pFunction)&DSmear::cmd_WashPoolDrain_Close);

    setFunctionMap("QueryTemperatureWet", (pFunction)&DSmear::cmd_QueryTemperatureWet);
    setFunctionMap("QueryPressurePN", (pFunction)&DSmear::cmd_QueryPressurePN);

    setFunctionMap("ComboLoop", (pFunction)&DSmear::cmd_ComboLoop);
}

void DSmear::stateInit()
{
    m_SmearCart_Lock = false;
}

bool DSmear::cmd_CleanLiquidSystem()
{
    SendComboActionStart("CleanLiquidSystem", QJsonValue(), 120000);
    return true;
}

bool DSmear::cmd_NewSlide1_ToAddSamplingPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("NewSlide1_ToAddSamplingPos");
        return true;
    }
    return false;
}

bool DSmear::cmd_NewSlide2_ToAddSamplingPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("NewSlide2_ToAddSamplingPos");
        return true;
    }
    return false;
}

bool DSmear::cmd_NewSlide1_ToPrintPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("NewSlide1_ToPrintPos");
        return true;
    }
    return false;
}

bool DSmear::cmd_NewSlide2_ToPrintPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("NewSlide2_ToPrintPos");
        return true;
    }
    return false;
}

bool DSmear::cmd_PrintSlideInfo()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("PrintSlideInfo");
        return true;
    }
    return false;
}

bool DSmear::cmd_PrintTextData()
{
    SendComboActionStart("PrintTextData");
    return true;
}

bool DSmear::cmd_PrintQRCode()
{
    SendComboActionStart("PrintQRCode");
    return true;
}

bool DSmear::cmd_PrintBarCode()
{
    SendComboActionStart("PrintBarCode");
    return true;
}

bool DSmear::cmd_Slide_FrPrintToAddSamplePos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("Slide_FrPrintToAddSamplePos");
        return true;
    }
    return false;
}

bool DSmear::cmd_Slide_FrPrintToStainCartPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("Slide_FrPrintToStainCartPos");
        return true;
    }
    return false;
}

bool DSmear::cmd_SmearCart_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_SmearCart_Lock == false) {
        m_SmearCart_Lock = true;
        SendComboActionStart("SmearCart_Reset");
        return true;
    }
    return false;
}

bool DSmear::cmd_Smear(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10};
    SendComboActionStart("Smear", arg);
    return true;
}

bool DSmear::cmd_Smear2(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {arg1, arg2, arg3, arg4, arg5};
    SendComboActionStart("Smear", arg);
    return true;
}

bool DSmear::cmd_FillWashPool()
{
    SendComboActionStart("FillWashPool");
    return true;
}

bool DSmear::cmd_CleanSmearBlade()
{
    SendComboActionStart("CleanSmearBlade");
    return true;
}

bool DSmear::cmd_CleanSmearBlade_Aging()
{
    SendComboActionStart("CleanSmearBlade_Aging");
    return true;
}

bool DSmear::cmd_CleanSmearBlade_Maintian()
{
    SendComboActionStart("CleanSmearBlade_Maintian");
    return true;
}

bool DSmear::cmd_WashPoolDrain_Open()
{
    SendComboActionStart("WashPoolDrain_Open");
    return true;
}

bool DSmear::cmd_WashPoolDrain_Close()
{
    SendComboActionStart("WashPoolDrain_Close");
    return true;
}

bool DSmear::cmd_SlideScanQRcode_Open(int msec)
{
    SendComboActionStart("SlideScanQRcode_Open", QJsonArray({msec}));
    return true;
}

bool DSmear::cmd_SlideScanQRcode_Close()
{
    SendComboActionStart("SlideScanQRcode_Close");
    return true;
}

bool DSmear::cmd_QueryTemperatureWet()
{
    SendCommand("QueryTemperatureWet");
    return true;
}

bool DSmear::cmd_QueryPressurePN()
{
    SendCommand("QueryPressurePN");
    return true;
}

bool DSmear::cmd_ComboLoop()
{
    SendComboActionStart("ComboLoop");
    return true;
}

bool DSmear::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (SmearCart_List.contains(api)) {
        m_SmearCart_Lock = false;
    }
    return RtDeviceBase::handleReceiveResult(api, resValue);
}

void DSmear::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (SmearCart_List.contains(api)) {
        m_SmearCart_Lock = false;
    }
    return RtDeviceBase::handleReceiveResultError(api, errorObj);
}


