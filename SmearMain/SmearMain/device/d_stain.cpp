#include "d_stain.h"

DStain::DStain(const QString &dev_id, QObject *parent)
    : RtDeviceBase(dev_id, parent)
{
    m_SlotFillFix_Lock = false;
    m_SlotFill_Lock = false;
    m_Drain_Lock = false;
    m_Wash_Lock = false;
    m_Recycle_Lock = false;
    m_RecycleBox_Lock = false;
    m_slot_fill_pos = -1;
    m_slot_drain_pos = -1;

    Drain_Lock_List << "Slot_Fix1_Drain" << "Slot_Fix2_Drain"
                    << "Slot_Drain_WithUp" << "Slot_Drain_WithoutUp" << "Drain_Waste_Tank";

    setFunctionMap("Perfuse_LiquidSystem", (pFunction)&DStain::cmd_Perfuse_LiquidSystem);

    setFunctionMap("G1", (pFunctionArg2)&DStain::cmd_G1);
    setFunctionMap("G2", (pFunctionArg2)&DStain::cmd_G2);
    setFunctionMap("G1_escape", (pFunctionArg2)&DStain::cmd_G1_escape);
    setFunctionMap("G1_withReset", (pFunctionArg2)&DStain::cmd_G1_withReset);
    setFunctionMap("G1_fromImport", (pFunctionArg1)&DStain::cmd_G1_fromImport);

    setFunctionMap("G1_Reset", (pFunction)&DStain::cmd_G1_Reset);
    setFunctionMap("G2_Reset", (pFunction)&DStain::cmd_G2_Reset);

    setFunctionMap("G1_stainonly1", (pFunctionArg2)&DStain::cmd_G1_stainonly1);
    setFunctionMap("G1_stainonly1_withReset", (pFunctionArg2)&DStain::cmd_G1_stainonly1_withReset);
    setFunctionMap("G1_stainonly2", (pFunctionArg2)&DStain::cmd_G1_stainonly2);
    setFunctionMap("G1_stainonly2_withReset", (pFunctionArg2)&DStain::cmd_G1_stainonly2_withReset);

    setFunctionMap("G2_escape", (pFunctionArg2)&DStain::cmd_G2_escape);
    setFunctionMap("G2_recycle", (pFunctionArg2)&DStain::cmd_G2_recycle);
    setFunctionMap("G2_recycle_reverse", (pFunctionArg2)&DStain::cmd_G2_recycle_reverse);

    setFunctionMap("G1_Open", (pFunction)&DStain::cmd_G1_Open);
    setFunctionMap("G1_Close", (pFunction)&DStain::cmd_G1_Close);
    setFunctionMap("G2_Open", (pFunction)&DStain::cmd_G2_Open);
    setFunctionMap("G2_Close", (pFunction)&DStain::cmd_G2_Close);

    setFunctionMap("NewRecycleBox", (pFunction)&DStain::cmd_NewRecycleBox);
    setFunctionMap("RecycleBox_ToDryPos", (pFunction)&DStain::cmd_RecycleBox_ToDryPos);
    setFunctionMap("RecycleBox_ToManualPos", (pFunction)&DStain::cmd_RecycleBox_ToManualPos);
    setFunctionMap("RecycleBox_ToReader", (pFunction)&DStain::cmd_RecycleBox_ToReader);

    setFunctionMap("Slot_Fix1_Fill", (pFunctionArg1)&DStain::cmd_Slot_Fix1_Fill);
    setFunctionMap("Slot_Fix2_Fill", (pFunctionArg1)&DStain::cmd_Slot_Fix2_Fill);
    setFunctionMap("Slot_Fix1_Drain", (pFunction)&DStain::cmd_Slot_Fix1_Drain);
    setFunctionMap("Slot_Fix2_Drain", (pFunction)&DStain::cmd_Slot_Fix2_Drain);

    setFunctionMap("Slot_Fill_A1", (pFunctionArg1)&DStain::cmd_Slot_Fill_A1);
    setFunctionMap("Slot_Fill_B1", (pFunctionArg1)&DStain::cmd_Slot_Fill_B1);
    setFunctionMap("Slot_Fill_A1B1", (pFunctionArg3)&DStain::cmd_Slot_Fill_A1B1);
    setFunctionMap("Slot_Fill_A2", (pFunctionArg1)&DStain::cmd_Slot_Fill_A2);
    setFunctionMap("Slot_Fill_B2", (pFunctionArg1)&DStain::cmd_Slot_Fill_B2);
    setFunctionMap("Slot_Fill_A2B2", (pFunctionArg3)&DStain::cmd_Slot_Fill_A2B2);
    setFunctionMap("Slot_Fill_Water", (pFunctionArg1)&DStain::cmd_Slot_Fill_Water);
    setFunctionMap("Slot_Fill_Alcohol", (pFunctionArg1)&DStain::cmd_Slot_Fill_Alcohol);

    setFunctionMap("Filling_Needle_Reset", (pFunction)&DStain::cmd_Filling_Needle_Reset);

    setFunctionMap("Drain_Needle_Reset", (pFunction)&DStain::cmd_Drain_Needle_Reset);
    setFunctionMap("Slot_Drain_WithUp", (pFunctionArg1)&DStain::cmd_Slot_Drain_WithUp);
    setFunctionMap("Slot_Drain_WithoutUp", (pFunctionArg1)&DStain::cmd_Slot_Drain_WithoutUp);
    setFunctionMap("Drain_Needle_Up", (pFunction)&DStain::cmd_Drain_Needle_Up);
    setFunctionMap("Drain_Needle_Drain", (pFunction)&DStain::cmd_Drain_Needle_Drain);

    setFunctionMap("Slot_Wash1_Wash", (pFunction)&DStain::cmd_Slot_Wash1_Wash);
    setFunctionMap("Slot_Wash2_Wash", (pFunction)&DStain::cmd_Slot_Wash2_Wash);
    setFunctionMap("Slot_Wash1_Fill", (pFunctionArg)&DStain::cmd_Slot_Wash1_Fill);
    setFunctionMap("Slot_Wash2_Fill", (pFunctionArg)&DStain::cmd_Slot_Wash2_Fill);
    setFunctionMap("Slot_Wash1_Drain", (pFunctionArg)&DStain::cmd_Slot_Wash1_Drain);
    setFunctionMap("Slot_Wash2_Drain", (pFunctionArg)&DStain::cmd_Slot_Wash2_Drain);

    setFunctionMap("Fill_Ret_Cup_With_Alcohol", (pFunction)&DStain::cmd_Fill_Ret_Cup_With_Alcohol);
    setFunctionMap("Fill_Blade_CleanPool_With_Alcohol", (pFunction)&DStain::cmd_Fill_Blade_CleanPool_With_Alcohol);

    setFunctionMap("Clean_Needles_With_Water", (pFunction)&DStain::cmd_Clean_Needles_With_Water);
    setFunctionMap("Clean_Needles_With_Alcohol", (pFunction)&DStain::cmd_Clean_Needles_With_Alcohol);
    setFunctionMap("Slot_Clean_With_Water", (pFunctionArg1)&DStain::cmd_Slot_Clean_With_Water);
    setFunctionMap("Slot_Clean_With_Alcohol", (pFunctionArg1)&DStain::cmd_Slot_Clean_With_Alcohol);

    setFunctionMap("Drain_Waste_Tank", (pFunction)&DStain::cmd_Drain_Waste_Tank);
    setFunctionMap("Drain_Ret_Cup", (pFunction)&DStain::cmd_Drain_Ret_Cup);
    setFunctionMap("Drain_Needles_Pool", (pFunction)&DStain::cmd_Drain_Needles_Pool);

    setFunctionMap("Perfuse_Water", (pFunction)&DStain::cmd_Perfuse_Water);
    setFunctionMap("Perfuse_Fix", (pFunction)&DStain::cmd_Perfuse_Fix);
    setFunctionMap("Perfuse_A1", (pFunction)&DStain::cmd_Perfuse_A1);
    setFunctionMap("Perfuse_B1", (pFunction)&DStain::cmd_Perfuse_B1);
    setFunctionMap("Perfuse_A2", (pFunction)&DStain::cmd_Perfuse_A2);
    setFunctionMap("Perfuse_B2", (pFunction)&DStain::cmd_Perfuse_B2);

    setFunctionMap("SlideStoreLight1_On", (pFunctionArg1)&DStain::cmd_SlideStoreLight1_On);
    setFunctionMap("SlideStoreLight1_Blink", (pFunctionArg1)&DStain::cmd_SlideStoreLight1_Blink);
    setFunctionMap("SlideStoreLight1_Off", (pFunction)&DStain::cmd_SlideStoreLight1_Off);
    setFunctionMap("SlideStoreLight2_On", (pFunctionArg1)&DStain::cmd_SlideStoreLight2_On);
    setFunctionMap("SlideStoreLight2_Blink", (pFunctionArg1)&DStain::cmd_SlideStoreLight2_Blink);
    setFunctionMap("SlideStoreLight2_Off", (pFunction)&DStain::cmd_SlideStoreLight2_Off);

    setFunctionMap("StainOnlyLight1_On", (pFunctionArg1)&DStain::cmd_StainOnlyLight1_On);
    setFunctionMap("StainOnlyLight1_Blink", (pFunctionArg1)&DStain::cmd_StainOnlyLight1_Blink);
    setFunctionMap("StainOnlyLight1_Off", (pFunction)&DStain::cmd_StainOnlyLight1_Off);
    setFunctionMap("StainOnlyLight2_On", (pFunctionArg1)&DStain::cmd_StainOnlyLight2_On);
    setFunctionMap("StainOnlyLight2_Blink", (pFunctionArg1)&DStain::cmd_StainOnlyLight2_Blink);
    setFunctionMap("StainOnlyLight2_Off", (pFunction)&DStain::cmd_StainOnlyLight2_Off);

    setFunctionMap("AirTank_Off", (pFunction)&DStain::cmd_AirTank_Off);
    setFunctionMap("AirTank_On", (pFunction)&DStain::cmd_AirTank_On);

    setFunctionMap("SetHeaterTempValue", (pFunctionArg3)&DStain::cmd_SetHeaterTempValue);
    setFunctionMap("OpenHeater", (pFunctionArg2)&DStain::cmd_OpenHeater);
    setFunctionMap("Heater_Open", (pFunction)&DStain::cmd_Heater_Open);
    setFunctionMap("Heater_Close", (pFunction)&DStain::cmd_Heater_Close);

    setFunctionMap("ComboLoop", (pFunction)&DStain::cmd_ComboLoop);

    setFunctionMap("QueryFloater", (pFunctionArg1)&DStain::cmd_QueryFloater);
    setFunctionMap("QueryAllFloater", (pFunctionArg1)&DStain::cmd_QueryAllFloater);
}

void DStain::stateInit()
{
    m_SlotFill_Lock = false;
    m_Drain_Lock = false;
    m_Wash_Lock = false;
    m_Recycle_Lock = false;
    m_RecycleBox_Lock = false;
}

bool DStain::cmd_Perfuse_LiquidSystem()
{
    SendComboActionStart("Perfuse_LiquidSystem", QJsonValue(), 120000);
    return true;
}

bool DStain::cmd_NewRecycleBox()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Recycle_Lock == false /*&& m_RecycleBox_Lock == false*/) {
        m_Recycle_Lock = true;
        SendComboActionStart("NewRecycleBox");
        return true;
    }
    return false;
}

bool DStain::cmd_RecycleBox_ToDryPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Recycle_Lock == false && m_RecycleBox_Lock == false) {
        m_Recycle_Lock = true;
        m_RecycleBox_Lock = true;
        SendComboActionStart("RecycleBox_ToDryPos");
        return true;
    }
    return false;
}

bool DStain::cmd_RecycleBox_ToManualPos()
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_RecycleBox_Lock == false) {
        m_RecycleBox_Lock = true;
        SendComboActionStart("RecycleBox_ToManualPos");
        return true;
    }
    return false;
}

bool DStain::cmd_RecycleBox_ToReader()
{
    if (isResetOk() == false) {
        return false;
    }

    if (m_RecycleBox_Lock == false) {
        m_RecycleBox_Lock = true;
        SendComboActionStart("RecycleBox_ToReader");
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fix1_Fill(int volume)
{
    if (m_SlotFillFix_Lock == false) {
        m_SlotFillFix_Lock = true;

        QJsonArray arg = {volume};
        SendComboActionStart("Slot_Fix1_Fill", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fix2_Fill(int volume)
{
    if (m_SlotFillFix_Lock == false) {
        m_SlotFillFix_Lock = true;

        QJsonArray arg = {volume};
        SendComboActionStart("Slot_Fix2_Fill", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fix1_Drain()
{
    if (m_Drain_Lock == false) {
        m_Drain_Lock = true;
        SendComboActionStart("Slot_Fix1_Drain");
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fix2_Drain()
{
    if (m_Drain_Lock == false) {
        m_Drain_Lock = true;
        SendComboActionStart("Slot_Fix2_Drain");
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_A1(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 2};
        SendComboActionStart("Slot_Fill_A1", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_B1(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 2};
        SendComboActionStart("Slot_Fill_B1", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_A1B1(int pos, int pa, int pb)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 2, pa, pb};
        SendComboActionStart("Slot_Fill_A1B1", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_A2(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 1};
        SendComboActionStart("Slot_Fill_A2", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_B2(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 1};
        SendComboActionStart("Slot_Fill_B2", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_A2B2(int pos, int pa, int pb)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos + 1, pa, pb};
        SendComboActionStart("Slot_Fill_A2B2", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_Water(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos && m_Wash_Lock == false) {
        m_SlotFill_Lock = true;
        m_Wash_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos};
        SendComboActionStart("Slot_Fill_Water", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Fill_Alcohol(int pos)
{
    if (m_SlotFill_Lock == false && m_slot_fill_pos != pos && m_slot_drain_pos != pos) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = pos;

        QJsonArray arg = {pos};
        SendComboActionStart("Slot_Fill_Alcohol", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Filling_Needle_Reset()
{
    if (m_SlotFill_Lock == false) {
        m_SlotFill_Lock = true;
        m_slot_fill_pos = -1;

        SendComboActionStart("Filling_Needle_Reset");
        return true;
    }
    return false;
}

bool DStain::cmd_Drain_Needle_Reset()
{
#if 1
    SendComboActionStart("Drain_Needle_Reset");
    m_slot_drain_pos = -1;
    return true;
#else
    if (m_Drain_Lock == false) {
        m_Drain_Lock = true;
        m_slot_drain_pos = -1;

        SendComboActionStart("Drain_Needle_Reset");
        return true;
    }
    return false;
#endif
}

bool DStain::cmd_Slot_Drain_WithUp(int pos)
{
    if (m_Drain_Lock == false && m_slot_drain_pos != pos && m_slot_fill_pos != pos) {
        m_Drain_Lock = true;
        m_slot_drain_pos = pos;

        QJsonArray arg = {pos};
        SendComboActionStart("Slot_Drain_WithUp", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Drain_WithoutUp(int pos)
{
    if (m_Drain_Lock == false && m_slot_drain_pos != pos && m_slot_fill_pos != pos) {
        m_Drain_Lock = true;
        m_slot_drain_pos = pos;

        QJsonArray arg = {pos};
        SendComboActionStart("Slot_Drain_WithoutUp", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_Drain_Needle_Up()
{
    SendComboActionStart("Drain_Needle_Up");
    return true;
}

bool DStain::cmd_Drain_Needle_Drain()
{
    SendComboActionStart("Drain_Needle_Drain");
    return true;
}

bool DStain::cmd_Slot_Wash1_Wash()
{
    if (m_Wash_Lock == false) {
        m_Wash_Lock = true;
        SendComboActionStart("Slot_Wash1_Wash");
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Wash2_Wash()
{
    if (m_Wash_Lock == false) {
        m_Wash_Lock = true;
        SendComboActionStart("Slot_Wash2_Wash");
        return true;
    }
    return false;
}

bool DStain::cmd_Slot_Wash1_Fill()
{
    SendComboActionStart("Slot_Wash1_Fill");
    return true;
}

bool DStain::cmd_Slot_Wash2_Fill()
{
    SendComboActionStart("Slot_Wash2_Fill");
    return true;
}

bool DStain::cmd_Slot_Wash1_Drain()
{
    SendComboActionStart("Slot_Wash1_Drain");
    return true;
}

bool DStain::cmd_Slot_Wash2_Drain()
{
    SendComboActionStart("Slot_Wash2_Drain");
    return true;
}

bool DStain::cmd_Fill_Ret_Cup_With_Alcohol()
{
    SendComboActionStart("Fill_Ret_Cup_With_Alcohol");
    return true;
}

bool DStain::cmd_Fill_Blade_CleanPool_With_Alcohol()
{
    SendComboActionStart("Fill_Blade_CleanPool_With_Alcohol");
    return true;
}

bool DStain::cmd_Clean_Needles_With_Water()
{
    SendComboActionStart("Clean_Needles_With_Water");
    return true;
}

bool DStain::cmd_Clean_Needles_With_Alcohol()
{
    SendComboActionStart("Clean_Needles_With_Alcohol");
    return true;
}

bool DStain::cmd_Slot_Clean_With_Water(int pos)
{
    QJsonArray arg = {pos};
    SendComboActionStart("Slot_Clean_With_Water", arg);
    return true;
}

bool DStain::cmd_Slot_Clean_With_Alcohol(int pos)
{
    QJsonArray arg = {pos};
    SendComboActionStart("Slot_Clean_With_Alcohol", arg);
    return true;
}

bool DStain::cmd_Drain_Waste_Tank()
{
    if (m_Drain_Lock == false) {
        m_Drain_Lock = true;
        SendComboActionStart("Drain_Waste_Tank");
        return true;
    }
    return false;
}

bool DStain::cmd_Drain_Ret_Cup()
{
    SendComboActionStart("Drain_Ret_Cup");
    return true;
}

bool DStain::cmd_Drain_Needles_Pool()
{
    SendComboActionStart("Drain_Needles_Pool");
    return true;
}

bool DStain::cmd_G1(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1", arg);
    return true;
}

bool DStain::cmd_G2(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G2", arg);
    return true;
}

bool DStain::cmd_G1_escape(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_escape", arg);
    return true;
}

bool DStain::cmd_G1_withReset(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_withReset", arg);
    return true;
}

bool DStain::cmd_G1_fromImport(int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {to};
    SendComboActionStart("G1_fromImport", arg);
    return true;
}

bool DStain::cmd_G1_Reset()
{
    SendComboActionStart("G1_Reset");
    return true;
}

bool DStain::cmd_G2_Reset()
{
    SendComboActionStart("G2_Reset");
    return true;
}

bool DStain::cmd_G2_escape(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G2_escape", arg);
    return true;
}

bool DStain::cmd_G2_recycle(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    if (m_Recycle_Lock == false) {
        m_Recycle_Lock = true;
        QJsonArray arg = {from, to};
        SendComboActionStart("G2_recycle", arg);
        return true;
    }
    return false;
}

bool DStain::cmd_G2_recycle_reverse(int from, int to)
{
    QJsonArray arg = {from, to};
    SendComboActionStart("G2_recycle_reverse", arg);
    return true;
}

bool DStain::cmd_G1_Open()
{
    SendComboActionStart("G1_Open");
    return true;
}

bool DStain::cmd_G1_Close()
{
    SendComboActionStart("G1_Close");
    return true;
}

bool DStain::cmd_G2_Open()
{
    SendComboActionStart("G2_Open");
    return true;
}

bool DStain::cmd_G2_Close()
{
    SendComboActionStart("G2_Close");
    return true;
}

bool DStain::cmd_G1_stainonly1(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_stainonly1", arg);
    return true;
}

bool DStain::cmd_G1_stainonly1_withReset(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_stainonly1_withReset", arg);
    return true;
}

bool DStain::cmd_G1_stainonly2(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_stainonly2", arg);
    return true;
}

bool DStain::cmd_G1_stainonly2_withReset(int from, int to)
{
    if (isResetOk() == false) {
        return false;
    }
    QJsonArray arg = {from, to};
    SendComboActionStart("G1_stainonly2_withReset", arg);
    return true;
}

bool DStain::cmd_Perfuse_Water()
{
    SendComboActionStart("Perfuse_Water");
    return true;
}

bool DStain::cmd_Perfuse_Fix()
{
    SendComboActionStart("Perfuse_Fix");
    return true;
}

bool DStain::cmd_Perfuse_A1()
{
    SendComboActionStart("Perfuse_A1");
    return true;
}

bool DStain::cmd_Perfuse_B1()
{
    SendComboActionStart("Perfuse_B1");
    return true;
}

bool DStain::cmd_Perfuse_A2()
{
    SendComboActionStart("Perfuse_A2");
    return true;
}

bool DStain::cmd_Perfuse_B2()
{
    SendComboActionStart("Perfuse_B2");
    return true;
}

bool DStain::cmd_SlideStoreLight1_On(int ryg)
{
    cmd_SlideStoreLight1_Off();

    QJsonArray arg;
    if (ryg == 0) {         //red
        arg = {1, 0, 0};
    } else if (ryg == 1) {  //yellow
        arg = {0, 0, 1};
    } else if (ryg == 2) {  //green
        arg = {0, 1, 0};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }

    SendComboActionStart("SlideStoreLight1_On", arg);
    return true;
}

bool DStain::cmd_SlideStoreLight1_Blink(int ryg)
{
    cmd_SlideStoreLight1_Off();

    QJsonArray arg;
    if (ryg == 0) {         //red
        arg = {3};
    } else if (ryg == 1) {  //yellow
        arg = {4};
    } else if (ryg == 2) {  //green
        arg = {5};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }
    SendComboActionStart("SlideStoreLight1_Blink", arg);
    return true;
}

bool DStain::cmd_SlideStoreLight1_Off()
{
    SendComboActionStart("SlideStoreLight1_Off");
    return true;
}

bool DStain::cmd_SlideStoreLight2_On(int ryg)
{
    cmd_SlideStoreLight2_Off();

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
    SendComboActionStart("SlideStoreLight2_On", arg);
    return true;
}

bool DStain::cmd_SlideStoreLight2_Blink(int ryg)
{
    cmd_SlideStoreLight2_Off();

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
    SendComboActionStart("SlideStoreLight2_Blink", arg);
    return true;
}

bool DStain::cmd_SlideStoreLight2_Off()
{
    SendComboActionStart("SlideStoreLight2_Off");
    return true;
}

bool DStain::cmd_StainOnlyLight1_On(int ryg)
{
    cmd_StainOnlyLight1_Off();

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
    SendComboActionStart("StainOnlyLight1_On", arg);
    return true;
}

bool DStain::cmd_StainOnlyLight1_Blink(int ryg)
{
    cmd_StainOnlyLight1_Off();

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
    SendComboActionStart("StainOnlyLight1_Blink", arg);
    return true;
}

bool DStain::cmd_StainOnlyLight1_Off()
{
    SendComboActionStart("StainOnlyLight1_Off");
    return true;
}

bool DStain::cmd_StainOnlyLight2_On(int ryg)
{
    cmd_StainOnlyLight2_Off();

    QJsonArray arg;
    if (ryg == 0) { //red
        arg = {1};
    } else if (ryg == 1) { //yellow
        arg = {3};
    } else if (ryg == 2) { //green
        arg = {2};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }
    SendComboActionStart("StainOnlyLight2_On", arg);
    return true;
}

bool DStain::cmd_StainOnlyLight2_Blink(int ryg)
{
    cmd_StainOnlyLight2_Off();

    QJsonArray arg;
    if (ryg == 0) { //red
        arg = {1};
    } else if (ryg == 1) { //yellow
        arg = {3};
    } else if (ryg == 2) { //green
        arg = {2};
    } else {
        qWarning() << __FUNCTION__ << "Arg Error!";
        return false;
    }
    SendComboActionStart("StainOnlyLight2_Blink", arg);
    return true;
}

bool DStain::cmd_StainOnlyLight2_Off()
{
    SendComboActionStart("StainOnlyLight2_Off");
    return true;
}

bool DStain::cmd_Beep_Off()
{
    SendComboActionStart("Beep_Off");
    return true;
}

bool DStain::cmd_Beep_On()
{
    cmd_Beep_Off();
    SendComboActionStart("Beep_On");
    return true;
}

bool DStain::cmd_Beep_OnMs(int msec)
{
    cmd_Beep_Off();
    SendComboActionStart("Beep_OnMs", QJsonArray({msec}));
    return true;
}

bool DStain::cmd_Beep_OnHz()
{
    cmd_Beep_Off();
    SendComboActionStart("Beep_OnHz");
    return true;
}

bool DStain::cmd_AirTank_Off()
{
    SendComboActionStart("AirTank_Off");
    return true;
}

bool DStain::cmd_AirTank_On()
{
    SendComboActionStart("AirTank_On");
    return true;
}

bool DStain::cmd_SetHeaterTempValue(int id, int temp, int threshold)
{
    QJsonObject obj;
    obj.insert("id", id);
    obj.insert("target", temp);
    obj.insert("threshold", threshold);

    SendCommand("SetHeaterTempValue", obj);
    return true;
}

bool DStain::cmd_OpenHeater(int id, bool on)
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

bool DStain::cmd_Heater_Open()
{
    SendComboActionStart("Heater_Open");
    return true;
}

bool DStain::cmd_Heater_Close()
{
    SendComboActionStart("Heater_Close");
    return true;
}

bool DStain::cmd_ComboLoop()
{
    SendComboActionStart("ComboLoop");
    return true;
}

bool DStain::cmd_QueryFloater(int id)
{
    QJsonObject obj;
    obj.insert("id", id);

    SendCommand("QueryAllFloater", obj);
    return true;
}

bool DStain::cmd_QueryAllFloater(int count)
{
    QJsonObject obj;
    obj.insert("count", count);

    SendCommand("QueryAllFloater", obj);
    return true;
}

bool DStain::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (api == "G2_recycle") {
        m_Recycle_Lock = false;
    } else if (api == "NewRecycleBox") {
        m_Recycle_Lock = false;
    } else if (api == "Slot_Fill_Water") {
        m_SlotFill_Lock = false;
        m_Wash_Lock = false;
        m_slot_fill_pos = -1;
    } else if (api == "Slot_Drain_WithUp"
               || api == "Slot_Drain_WithoutUp"
               || api == "Drain_Needle_Reset") {
        m_Drain_Lock = false;
        m_slot_drain_pos = -1;
    } else if (api == "Slot_Wash1_Wash"
               || api == "Slot_Wash2_Wash"
               || api == "Slot_Fill_Water") {
        m_Wash_Lock = false;
    } else if (api == "Slot_Fix1_Fill"
               || api == "Slot_Fix2_Fill") {
        m_SlotFillFix_Lock = false;
    } else if (Drain_Lock_List.contains(api)) {
        /* "Slot_Fix1_Drain" << "Slot_Fix2_Drain" <<
         * "Slot_Drain_WithUp" << "Slot_Drain_WithoutUp"
         * << "Drain_Waste_Tank"; */
        m_Drain_Lock = false;
    } else if (api.startsWith("Slot_Fill_")) {
        /* Slot_Fill_A1 Slot_Fill_B1 Slot_Fill_A1B1
         * Slot_Fill_A2 Slot_Fill_B2 Slot_Fill_A2B2
         * Slot_Fill_Water Slot_Fill_Alcohol */
        m_SlotFill_Lock = false;
        m_slot_fill_pos = -1;
    } else if (api.startsWith("RecycleBox_")) {
        m_RecycleBox_Lock = false;
        if (api == "RecycleBox_ToDryPos") {
            m_Recycle_Lock = false;
        }
    }
    return RtDeviceBase::handleReceiveResult(api, resValue);
}

void DStain::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (api == "G2_recycle") {
        m_Recycle_Lock = false;
    } else if (api == "NewRecycleBox") {
        m_Recycle_Lock = false;
    } else if (api == "Slot_Fill_Water") {
        m_SlotFill_Lock = false;
        m_Wash_Lock = false;
        m_slot_fill_pos = -1;
    } else if (api == "Slot_Drain_WithUp"
               || api == "Slot_Drain_WithoutUp"
               || api == "Drain_Needle_Reset") {
        m_Drain_Lock = false;
        m_slot_drain_pos = -1;
    } else if (api == "Slot_Wash1_Wash"
               || api == "Slot_Wash2_Wash"
               || api == "Slot_Fill_Water") {
        m_Wash_Lock = false;
    } else if (api == "Slot_Fix1_Fill"
               || api == "Slot_Fix2_Fill") {
        m_SlotFillFix_Lock = false;
    } else if (Drain_Lock_List.contains(api)) {
        /* "Slot_Fix1_Drain" << "Slot_Fix2_Drain" <<
         * "Slot_Drain_WithUp" << "Slot_Drain_WithoutUp"
         * << "Drain_Waste_Tank"; */
        m_Drain_Lock = false;
    } else if (api.startsWith("Slot_Fill_")) {
        /* Slot_Fill_A1 Slot_Fill_B1 Slot_Fill_A1B1
         * Slot_Fill_A2 Slot_Fill_B2 Slot_Fill_A2B2
         * Slot_Fill_Water Slot_Fill_Alcohol */
        m_SlotFill_Lock = false;
        m_slot_fill_pos = -1;
    } else if (api.startsWith("RecycleBox_")) {
        m_RecycleBox_Lock = false;
        if (api == "RecycleBox_ToDryPos") {
            m_Recycle_Lock = false;
        }
    }
    return RtDeviceBase::handleReceiveResultError(api, errorObj);
}
