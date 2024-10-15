#include "d_sample.h"

DSample::DSample(const QString &dev_id, QObject *parent)
    : RtDeviceBase(dev_id, parent)
{
    m_Gripper_Lock = false;
    m_isAirCompressorOpen = true;

    Gripper_List << "GetNewTube"
                 << "TubeFrGripToMix" << "TubeFrMixToCart"
                 << "ReturnTubeFrCart" << "ReturnTubeFrMix";

    setFunctionMap("CleanLiquidSystem", (pFunction)&DSample::cmd_CleanLiquidSystem);

    setFunctionMap("RotateTube", (pFunction)&DSample::cmd_RotateTube);

    setFunctionMap("Gripper_Open", (pFunction)&DSample::cmd_Gripper_Open);
    setFunctionMap("Gripper_Close", (pFunction)&DSample::cmd_Gripper_Close);

    setFunctionMap("GetNewTube", (pFunctionArg1)&DSample::cmd_GetNewTube);
    setFunctionMap("TubeFrGripToMix", (pFunction)&DSample::cmd_TubeFrGripToMix);
    setFunctionMap("TubeFrGripToCart", (pFunction)&DSample::cmd_TubeFrGripToCart);
    setFunctionMap("MixTube", (pFunction)&DSample::cmd_MixTube);

    setFunctionMap("TubeFrMixToCart", (pFunction)&DSample::cmd_TubeFrMixToCart);
    setFunctionMap("CartToSamplePos_UnCapping", (pFunction)&DSample::cmd_CartToSamplePos_UnCapping);
    setFunctionMap("CartToExitPos_Capping", (pFunction)&DSample::cmd_CartToExitPos_Capping);
    setFunctionMap("CartToSamplePos", (pFunction)&DSample::cmd_CartToSamplePos);
    setFunctionMap("CartToExitPos", (pFunction)&DSample::cmd_CartToExitPos);
    setFunctionMap("CartToExitPos_Capping2", (pFunction)&DSample::cmd_CartToExitPos_Capping2);
    setFunctionMap("CartToCapDetect", (pFunction)&DSample::cmd_CartToCapDetect);

    setFunctionMap("ReturnTubeFrCart", (pFunctionArg2)&DSample::cmd_ReturnTubeFrCart);
    setFunctionMap("ReturnTubeFrMix", (pFunction)&DSample::cmd_ReturnTubeFrMix);

    setFunctionMap("NeedleAdd_TakeSample", (pFunctionArg1)&DSample::cmd_NeedleAdd_TakeSample);
    setFunctionMap("NeedleAdd_TakeSample_Mini", (pFunction)&DSample::cmd_NeedleAdd_TakeSample_Mini);
    setFunctionMap("NeedleAdd_TakeSample_To_BladePool", (pFunction)&DSample::cmd_NeedleAdd_TakeSample_To_BladePool);

    setFunctionMap("AddSample_Normal", (pFunctionArg1)&DSample::cmd_AddSample_Normal);
    setFunctionMap("Take_Mix_Ret", (pFunctionArg1)&DSample::cmd_Take_Mix_Ret);
    setFunctionMap("RetMixSample", (pFunctionArg2)&DSample::cmd_RetMixSample);
    setFunctionMap("PoolRet_Fill", (pFunction)&DSample::cmd_PoolRet_Fill);

    setFunctionMap("Clean_AddNeedle", (pFunction)&DSample::cmd_Clean_AddNeedle);
    setFunctionMap("Clean_AddNeedle_Maintain", (pFunction)&DSample::cmd_Clean_AddNeedle_Maintain);
    setFunctionMap("Clean_AddNeedle_Ret", (pFunction)&DSample::cmd_Clean_AddNeedle_Ret);

    setFunctionMap("StainCart_Reset", (pFunction)&DSample::cmd_StainCart_Reset);
    setFunctionMap("StainCart_LoadSlide", (pFunction)&DSample::cmd_StainCart_LoadSlide);
    setFunctionMap("StainCart_ToStainImport", (pFunctionArg1)&DSample::cmd_StainCart_ToStainImport);
    setFunctionMap("StainCart_ToStainImportWithoutHeat", (pFunction)&DSample::cmd_StainCart_ToStainImportWithoutHeat);

    setFunctionMap("SlideStoreCleanOpen", (pFunctionArg1)&DSample::cmd_SlideStoreCleanOpen);
    setFunctionMap("SlideStoreCleanClose", (pFunctionArg1)&DSample::cmd_SlideStoreCleanClose);
    setFunctionMap("DrainFilter", (pFunction)&DSample::cmd_DrainFilter);
    setFunctionMap("Perfuse_Diluent_Tube", (pFunction)& DSample::cmd_Perfuse_Diluent_Tube);
    setFunctionMap("Perfuse_Maintain_Blade_Pool", (pFunction)& DSample::cmd_Perfuse_Maintain_Blade_Pool);
    setFunctionMap("Soak_Needle_Maintain", (pFunction)& DSample::cmd_Soak_Needle_Maintain);

    setFunctionMap("MainLight_On", (pFunctionArg1)&DSample::cmd_MainLight_On);
    setFunctionMap("MainLight_Blink", (pFunctionArg1)&DSample::cmd_MainLight_Blink);
    setFunctionMap("MainLight_Breathe", (pFunction)&DSample::cmd_MainLight_Breathe);
    setFunctionMap("MainLight_Off", (pFunction)&DSample::cmd_MainLight_Off);

    setFunctionMap("SetHeaterTempValue", (pFunctionArg3)&DSample::cmd_SetHeaterTempValue);
    setFunctionMap("OpenHeater", (pFunctionArg2)&DSample::cmd_OpenHeater);
    setFunctionMap("Heater_Open", (pFunction)&DSample::cmd_Heater_Open);
    setFunctionMap("Heater_Close", (pFunction)&DSample::cmd_Heater_Close);

    setFunctionMap("AirCompressor_Open", (pFunctionArg)&DSample::cmd_AirCompressor_Open);
    setFunctionMap("AirCompressor_Close", (pFunctionArg)&DSample::cmd_AirCompressor_Close);

    setFunctionMap("ComboLoop", (pFunction)&DSample::cmd_ComboLoop);

    setFunctionMap("QueryFloater", (pFunctionArg1)&DSample::cmd_QueryFloater);
    setFunctionMap("QueryAllFloater", (pFunctionArg1)&DSample::cmd_QueryAllFloater);
}

bool DSample::cmd_CleanLiquidSystem()
{
    SendComboActionStart("CleanLiquidSystem", QJsonValue(), 120000);
    return true;
}

bool DSample::cmd_RotateTube()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("RotateTube");
    return true;
}

bool DSample::cmd_Gripper_Open()
{
    SendComboActionStart("Gripper_Open");
    return true;
}

bool DSample::cmd_Gripper_Close()
{
    SendComboActionStart("Gripper_Close");
    return true;
}

bool DSample::cmd_GetNewTube(int pos)
{
    /* pos : (0:rack, 1:closet) */
    if (isResetOk() == false) {
        return false;
    }
    if (m_Gripper_Lock == false) {
        m_Gripper_Lock = true;
        QJsonArray arg = {pos};
        SendComboActionStart("GetNewTube", arg);
        return true;
    }
    return false;
}

bool DSample::cmd_TubeFrGripToMix()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Gripper_Lock == false) {
        m_Gripper_Lock = true;
        SendComboActionStart("TubeFrGripToMix");
        return true;
    }
    return false;
}

bool DSample::cmd_TubeFrGripToCart()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("TubeFrGripToCart");
    return true;
}

bool DSample::cmd_MixTube()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("MixTube");
    return true;
}

bool DSample::cmd_TubeFrMixToCart()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Gripper_Lock == false) {
        m_Gripper_Lock = true;
        SendComboActionStart("TubeFrMixToCart");
        return true;
    }
    return false;
}

bool DSample::cmd_CartToSamplePos_UnCapping()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToSamplePos_UnCapping");
    return true;
}

bool DSample::cmd_CartToExitPos_Capping()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToExitPos_Capping");
    return true;
}

bool DSample::cmd_CartToSamplePos()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToSamplePos");
    return true;
}

bool DSample::cmd_CartToExitPos()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToExitPos");
    return true;
}

bool DSample::cmd_CartToExitPos_Capping2()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToExitPos_Capping2");
    return true;
}

bool DSample::cmd_CartToCapDetect()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("CartToCapDetect");
    return true;
}

/* pos0 : (2:normal, 3:mini) pos1 : (0:rack, 1:closet) */
bool DSample::cmd_ReturnTubeFrCart(int pos0)
{ 
    if (isResetOk() == false) {
        return false;
    }
    if (m_Gripper_Lock == false) {
        m_Gripper_Lock = true;
        QJsonArray arg = {pos0};
        SendComboActionStart("ReturnTubeFrCart", arg);
        return true;
    }
    return false;
}

bool DSample::cmd_ReturnTubeFrMix(int pos0)
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Gripper_Lock == false) {
        m_Gripper_Lock = true;
        QJsonArray arg = {pos0};
        SendComboActionStart("ReturnTubeFrMix", arg);
        return true;
    }
    return false;
}

bool DSample::cmd_NeedleAdd_TakeSample(int volume)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {volume};
    SendComboActionStart("NeedleAdd_TakeSample", arg);
    return true;
}

bool DSample::cmd_NeedleAdd_TakeSample_Mini(int volume)
{
    QJsonArray arg = {volume};
    SendComboActionStart("NeedleAdd_TakeSample_Mini", arg);
    return true;
}

bool DSample::cmd_NeedleAdd_TakeSample_To_BladePool()
{
    SendComboActionStart("NeedleAdd_TakeSample_To_BladePool");
    return true;
}

bool DSample::cmd_AddSample_Normal(int volume)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {volume};
    SendComboActionStart("AddSample_Normal", arg);
    return true;
}

bool DSample::cmd_Take_Mix_Ret(int volume)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {volume};
    SendComboActionStart("Take_Mix_Ret", arg);
    return true;
}

bool DSample::cmd_RetMixSample(int inout, int add_volume)
{
    QJsonArray arg = {inout, add_volume};
    SendComboActionStart("RetMixSample", arg);
    return true;
}

bool DSample::cmd_PoolRet_Fill()
{
    SendComboActionStart("PoolRet_Fill");
    return true;
}

bool DSample::cmd_Clean_AddNeedle()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Clean_AddNeedle");
    return true;
}

bool DSample::cmd_Clean_AddNeedle_Maintain()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Clean_AddNeedle_Maintain");
    return true;
}

bool DSample::cmd_Clean_AddNeedle_Ret()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("Clean_AddNeedle_Ret");
    return true;
}

bool DSample::cmd_StainCart_Reset()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("StainCart_Reset");
    return true;
}

bool DSample::cmd_StainCart_LoadSlide()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("StainCart_LoadSlide");
    return true;
}

bool DSample::cmd_StainCart_ToStainImport(int msec)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {msec};
    SendComboActionStart("StainCart_ToStainImport", arg);
    return true;
}

bool DSample::cmd_StainCart_ToStainImportWithoutHeat()
{
    if (isResetOk() == false) {
        return false;
    }
    SendComboActionStart("StainCart_ToStainImportWithoutHeat");
    return true;
}

bool DSample::cmd_SlideStoreCleanOpen(int index)
{
    QJsonArray arg = {index};
    SendComboActionStart("SlideStoreCleanOpen", arg);
    return true;
}

bool DSample::cmd_SlideStoreCleanClose(int index)
{
    QJsonArray arg = {index};
    SendComboActionStart("SlideStoreCleanClose", arg);
    return true;
}

bool DSample::cmd_DrainFilter()
{
    SendComboActionStart("DrainFilter");
    return true;
}

bool DSample::cmd_Perfuse_Diluent_Tube()
{
    SendComboActionStart("Perfuse_Diluent_Tube");
    return true;
}

bool DSample::cmd_Perfuse_Maintain_Blade_Pool()
{
    SendComboActionStart("Perfuse_Maintain_Blade_Pool");
    return true;
}

bool DSample::cmd_Soak_Needle_Maintain()
{
    SendComboActionStart("Soak_Needle_Maintain");
    return true;
}

bool DSample::cmd_MainLight_On(int ryg)
{
    cmd_MainLight_Off();

    QJsonArray arg;
    if (ryg == 0) { //red
        arg = {1, 0, 0};
    } else if (ryg == 1) { //yellow
        arg = {0, 0, 1};
    } else if (ryg == 2) { //green
        arg = {0, 1, 0};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }

    SendComboActionStart("MainLight_On", arg);
    return true;
}

bool DSample::cmd_MainLight_Blink(int ryg)
{
    cmd_MainLight_Off();

    QJsonArray arg;
    if (ryg == 0) { //red
        arg = {3};
    } else if (ryg == 1) { //yellow
        arg = {4};
    } else if (ryg == 2) { //green
        arg = {5};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }

    SendComboActionStart("MainLight_Blink", arg);
    return true;
}

bool DSample::cmd_MainLight_Breathe()
{
    cmd_MainLight_Off();

    SendComboActionStart("MainLight_Breathe");
    return true;
}

bool DSample::cmd_MainLight_Off()
{
    SendComboActionStart("MainLight_Off");
    return true;
}

bool DSample::cmd_SetHeaterTempValue(int id, int temp, int threshold)
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("target", temp);
    obj.insert("threshold", threshold);

    SendCommand("SetHeaterTempValue", obj);
    return true;
}

bool DSample::cmd_OpenHeater(int id, bool on)
{
    QJsonObject obj;
    obj.insert("id", id);

    if (on) {
        SendCommand("OpenHeater", obj);
    } else {
        SendCommand("CloseHeater", obj);
    }
    return true;
}

bool DSample::cmd_Heater_Open()
{
    SendComboActionStart("Heater_Open");
    return true;
}

bool DSample::cmd_Heater_Close()
{
    SendComboActionStart("Heater_Close");
    return true;
}

bool DSample::cmd_AirCompressor_Open()
{
    if (m_isAirCompressorOpen == false) {
        SendComboActionStart("AirCompressor_Open");
        return true;
    }
    return false;
}

bool DSample::cmd_AirCompressor_Close()
{
    if (m_isAirCompressorOpen == true) {
        SendComboActionStart("AirCompressor_Close");
        return true;
    }
    return false;
}

bool DSample::cmd_ComboLoop()
{
    SendComboActionStart("ComboLoop");
    return true;
}

bool DSample::cmd_QueryFloater(int id)
{
    QJsonObject obj;
    obj.insert("id", id);

    SendCommand("QueryAllFloater", obj);
    return true;
}

bool DSample::cmd_QueryAllFloater(int count)
{
    QJsonObject obj;
    obj.insert("count", count);

    SendCommand("QueryAllFloater", obj);
    return true;
}

bool DSample::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (Gripper_List.contains(api)) {
        m_Gripper_Lock = false;
    } else if (api == "AirCompressor_Open") {
        m_isAirCompressorOpen = true;
    } else if (api == "AirCompressor_Close") {
        m_isAirCompressorOpen = false;
    }
    return RtDeviceBase::handleReceiveResult(api, resValue);
}

void DSample::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (Gripper_List.contains(api)) {
        m_Gripper_Lock = false;
    }
    return RtDeviceBase::handleReceiveResultError(api, errorObj);
}
