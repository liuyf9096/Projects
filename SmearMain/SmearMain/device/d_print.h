#ifndef D_PRINT_H
#define D_PRINT_H

#include "rt_device_base.h"

class DPrint : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DPrint(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DPrint() {}

    virtual bool cmd_Reset() override;

    bool cmd_PrintHeadDown();
    bool cmd_PrintTextData();
    bool cmd_PrintQRCode();
    bool cmd_PrintBarCode();
    bool cmd_PrintHeadUp();

    bool cmd_PrinterContentSetting(int row, const QString &content);
    bool cmd_PrinterBarcodeSetting(const QString &code);
    bool cmd_PrinterQRcodeSetting(const QString &code);
    bool cmd_SetPrinterMode(int mode);

    bool cmd_SetPrinterQRCodeType(int type);
    bool cmd_SetPrinterBarCodeType(int type);
};

#endif // D_PRINT_H
