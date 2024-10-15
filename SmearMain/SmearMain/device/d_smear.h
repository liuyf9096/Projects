#ifndef D_SMEAR_H
#define D_SMEAR_H

#include "rt_device_base.h"

// #3 board
class DSmear : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DSmear(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DSmear() {}

    virtual void stateInit() override;

    bool cmd_CleanLiquidSystem();

    bool cmd_NewSlide1_ToAddSamplingPos();
    bool cmd_NewSlide2_ToAddSamplingPos();
    bool cmd_NewSlide1_ToPrintPos();
    bool cmd_NewSlide2_ToPrintPos();

    bool cmd_PrintSlideInfo();
    bool cmd_PrintTextData();
    bool cmd_PrintQRCode();
    bool cmd_PrintBarCode();

    bool cmd_Slide_FrPrintToAddSamplePos();
    bool cmd_Slide_FrPrintToStainCartPos();
    bool cmd_SmearCart_Reset();

    /* arg1 :z_hight */
    /* arg2 :start_pos */
    /* arg3 :expend_time */
    /* arg4 :start_speed */
    /* arg5 :max_speed */
    /* arg6 :length:2200 */
    /* arg7 :z_wait:700 */
    /* arg8 :z_start_speed:2000 */
    /* arg9 :z_max_speed:6000 */
    /* arg10:z_range:2000 */
    bool cmd_Smear(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);
    bool cmd_Smear2(int arg1, int arg2, int arg3, int arg4, int arg5);

    bool cmd_FillWashPool();
    bool cmd_CleanSmearBlade();
    bool cmd_CleanSmearBlade_Aging();
    bool cmd_CleanSmearBlade_Maintian();

    bool cmd_WashPoolDrain_Open();
    bool cmd_WashPoolDrain_Close();

    bool cmd_SlideScanQRcode_Open(int msec);
    bool cmd_SlideScanQRcode_Close();

    bool cmd_QueryTemperatureWet();
    bool cmd_QueryPressurePN();

    bool cmd_ComboLoop();

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue) override;
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj) override;

private:
    bool m_SmearCart_Lock;
    QStringList SmearCart_List;
};

#endif // D_SMEAR_H

