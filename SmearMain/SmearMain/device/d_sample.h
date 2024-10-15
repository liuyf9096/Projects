#ifndef D_SAMPLE_H
#define D_SAMPLE_H

#include "rt_device_base.h"

// #1 board
class DSample : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DSample(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DSample() {}

    bool cmd_CleanLiquidSystem();

    bool cmd_RotateTube();

//! [1] pos : (0:rack, 1:closet)]
    bool cmd_GetNewTube(int pos);   //210 p0:(0:rack, 1:closet)
//! [1] pos : (0:rack, 1:closet)]

    bool cmd_TubeFrGripToMix(); //211
    bool cmd_TubeFrGripToCart();//212
    bool cmd_TubeFrMixToCart(); //214
    bool cmd_MixTube();         //219

    bool cmd_CartToSamplePos_UnCapping();   //220
    bool cmd_CartToExitPos_Capping();       //221
    bool cmd_CartToSamplePos();             //222
    bool cmd_CartToExitPos();               //223
    bool cmd_CartToExitPos_Capping2();      //224
    bool cmd_CartToCapDetect();             //225

    bool cmd_ReturnTubeFrCart(int pos0);    //250 p0:(0:rack, 1:closet)
    bool cmd_ReturnTubeFrMix(int pos0);     //254 p0:(0:rack, 1:closet)
    bool cmd_Gripper_Open();    //251
    bool cmd_Gripper_Close();   //252

    bool cmd_NeedleAdd_TakeSample(int volume);
    bool cmd_NeedleAdd_TakeSample_Mini(int volume);
    bool cmd_NeedleAdd_TakeSample_To_BladePool();

    bool cmd_AddSample_Normal(int volume);
    bool cmd_RetMixSample(int inout, int add_volume);
    bool cmd_Take_Mix_Ret(int volume);
    bool cmd_PoolRet_Fill();

    bool cmd_Clean_AddNeedle();
    bool cmd_Clean_AddNeedle_Maintain();
    bool cmd_Clean_AddNeedle_Ret();

    bool cmd_StainCart_Reset();
    bool cmd_StainCart_LoadSlide();
    bool cmd_StainCart_ToStainImport(int msec = 5000);
    bool cmd_StainCart_ToStainImportWithoutHeat();

    bool cmd_SlideStoreCleanOpen(int index);
    bool cmd_SlideStoreCleanClose(int index);
    bool cmd_DrainFilter();
    bool cmd_Perfuse_Diluent_Tube();
    bool cmd_Perfuse_Maintain_Blade_Pool();
    bool cmd_Soak_Needle_Maintain();

    bool cmd_MainLight_On(int ryg);
    bool cmd_MainLight_Blink(int ryg);
    bool cmd_MainLight_Breathe();
    bool cmd_MainLight_Off();

    bool cmd_SetHeaterTempValue(int id, int temp, int threshold = 200);
    bool cmd_OpenHeater(int id, bool on);
    bool cmd_Heater_Open();
    bool cmd_Heater_Close();

    bool cmd_AirCompressor_Open();
    bool cmd_AirCompressor_Close();

    bool cmd_ComboLoop();

    bool cmd_QueryFloater(int id);
    bool cmd_QueryAllFloater(int count);

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue) override;
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj) override;

private:
    bool m_Gripper_Lock;
    bool m_isAirCompressorOpen;
    QStringList Gripper_List;
};

#endif // D_SAMPLE_H
