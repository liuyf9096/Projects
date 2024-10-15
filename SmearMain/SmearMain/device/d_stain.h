   #ifndef D_STAIN_H
#define D_STAIN_H

#include "rt_device_base.h"

// #2 board
class DStain : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DStain(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DStain() {}

    virtual void stateInit() override;

    bool cmd_Perfuse_LiquidSystem();

    /* Gripper */
    bool cmd_G1(int from, int to);
    bool cmd_G2(int from, int to);
    bool cmd_G1_escape(int from, int to);
    bool cmd_G1_withReset(int from, int to);
    bool cmd_G1_fromImport(int to);

    bool cmd_G1_Reset();
    bool cmd_G2_Reset();

    bool cmd_G1_stainonly1(int from, int to);
    bool cmd_G1_stainonly1_withReset(int from, int to);
    bool cmd_G1_stainonly2(int from, int to);
    bool cmd_G1_stainonly2_withReset(int from, int to);

    bool cmd_G2_escape(int from, int to);
    bool cmd_G2_recycle(int from, int to);
    bool cmd_G2_recycle_reverse(int from, int to);

    bool cmd_G1_Open();
    bool cmd_G1_Close();
    bool cmd_G2_Open();
    bool cmd_G2_Close();

    /* Recycle Box */
    bool cmd_NewRecycleBox();
    bool cmd_RecycleBox_ToDryPos();
    bool cmd_RecycleBox_ToManualPos();
    bool cmd_RecycleBox_ToReader();

    /* Slot-Fix */
    bool cmd_Slot_Fix1_Fill(int volume);
    bool cmd_Slot_Fix2_Fill(int volume);
    bool cmd_Slot_Fix1_Drain();
    bool cmd_Slot_Fix2_Drain();

    /* Solution A,B */
    bool cmd_Slot_Fill_A1(int pos);
    bool cmd_Slot_Fill_B1(int pos);
    bool cmd_Slot_Fill_A1B1(int pos, int pa, int pb);
    bool cmd_Slot_Fill_A2(int pos);
    bool cmd_Slot_Fill_B2(int pos);
    bool cmd_Slot_Fill_A2B2(int pos, int pa, int pb);
    bool cmd_Slot_Fill_Water(int pos);
    bool cmd_Slot_Fill_Alcohol(int pos);

    bool cmd_Filling_Needle_Reset();

    /* Drain Needle */
    bool cmd_Drain_Needle_Reset();
    bool cmd_Slot_Drain_WithUp(int pos);
    bool cmd_Slot_Drain_WithoutUp(int pos);
    bool cmd_Drain_Needle_Up();
    bool cmd_Drain_Needle_Drain();

    /* Slot-Wash */
    bool cmd_Slot_Wash1_Wash();
    bool cmd_Slot_Wash2_Wash();
    bool cmd_Slot_Wash1_Fill();
    bool cmd_Slot_Wash2_Fill();
    bool cmd_Slot_Wash1_Drain();
    bool cmd_Slot_Wash2_Drain();

    /* Ret */
    bool cmd_Fill_Ret_Cup_With_Alcohol();
    bool cmd_Fill_Blade_CleanPool_With_Alcohol();

    /* Clean */
    bool cmd_Clean_Needles_With_Water();
    bool cmd_Clean_Needles_With_Alcohol();
    bool cmd_Slot_Clean_With_Water(int pos);
    bool cmd_Slot_Clean_With_Alcohol(int pos);

    bool cmd_Drain_Waste_Tank();
    bool cmd_Drain_Ret_Cup();
    bool cmd_Drain_Needles_Pool();

    /* maintain */
    bool cmd_Perfuse_Water();
    bool cmd_Perfuse_Fix();
    bool cmd_Perfuse_A1();
    bool cmd_Perfuse_B1();
    bool cmd_Perfuse_A2();
    bool cmd_Perfuse_B2();

    /* Light */
    /* Red:0 Yellow:1 Green:2 */
    bool cmd_SlideStoreLight1_On(int ryg);
    bool cmd_SlideStoreLight1_Blink(int ryg);
    bool cmd_SlideStoreLight1_Off();

    bool cmd_SlideStoreLight2_On(int ryg);
    bool cmd_SlideStoreLight2_Blink(int ryg);
    bool cmd_SlideStoreLight2_Off();

    bool cmd_StainOnlyLight1_On(int ryg);
    bool cmd_StainOnlyLight1_Blink(int ryg);
    bool cmd_StainOnlyLight1_Off();

    bool cmd_StainOnlyLight2_On(int ryg);
    bool cmd_StainOnlyLight2_Blink(int ryg);
    bool cmd_StainOnlyLight2_Off();

    /* Beep */
    bool cmd_Beep_Off();
    bool cmd_Beep_On();
    bool cmd_Beep_OnMs(int msec);
    bool cmd_Beep_OnHz();

    bool cmd_AirTank_Off();
    bool cmd_AirTank_On();

    bool cmd_SetHeaterTempValue(int id, int temp, int threshold = 200);
    bool cmd_OpenHeater(int id, bool on);
    bool cmd_Heater_Open();
    bool cmd_Heater_Close();

    bool cmd_ComboLoop();

    bool cmd_QueryFloater(int id);
    bool cmd_QueryAllFloater(int count);

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue) override;
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj) override;

private:
    bool m_SlotFillFix_Lock;
    bool m_SlotFill_Lock;
    bool m_Drain_Lock;
    bool m_Wash_Lock;
    bool m_Recycle_Lock;
    bool m_RecycleBox_Lock;
    int m_slot_fill_pos;
    int m_slot_drain_pos;

    QStringList Drain_Lock_List;
};

#endif // D_STAIN_H
