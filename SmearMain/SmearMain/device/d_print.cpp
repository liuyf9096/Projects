#include "d_print.h"

DPrint::DPrint(const QString &dev_id, QObject *parent)
    : RtDeviceBase(dev_id, parent)
{
    setFunctionMap("PrinterContentSetting", (pFunctionArgIS)&DPrint::cmd_PrinterContentSetting);
    setFunctionMap("PrinterBarcodeSetting", (pFunctionArgS)&DPrint::cmd_PrinterBarcodeSetting);
    setFunctionMap("PrinterQRcodeSetting", (pFunctionArgS)&DPrint::cmd_PrinterQRcodeSetting);
    setFunctionMap("SetPrinterMode", (pFunctionArg1)&DPrint::cmd_SetPrinterMode);
}

bool DPrint::cmd_Reset()
{
    SendCommand("PrinterReset");
    return true;
}

bool DPrint::cmd_PrintHeadDown()
{
    SendCommand("PrintHeadDown");
    return true;
}

bool DPrint::cmd_PrintTextData()
{
    SendCommand("PrintTextData");
    return true;
}

bool DPrint::cmd_PrintQRCode()
{
    SendCommand("PrintQRCode");
    return true;
}

bool DPrint::cmd_PrintBarCode()
{
    SendCommand("PrintBarCode");
    return true;
}

bool DPrint::cmd_PrintHeadUp()
{
    SendCommand("PrintHeadUp");
    return true;
}

bool DPrint::cmd_PrinterContentSetting(int row, const QString &content)
{
    QJsonObject obj;
    obj.insert("row", row);
    obj.insert("content", content);

    SendCommand("PrinterContentSetting", obj);
    return true;
}

bool DPrint::cmd_PrinterBarcodeSetting(const QString &code)
{
    QJsonObject obj;
    obj.insert("code", code);

    SendCommand("PrinterBarcodeSetting", obj);
    return true;
}

bool DPrint::cmd_PrinterQRcodeSetting(const QString &code)
{
    QJsonObject obj;
    obj.insert("code", code);

    SendCommand("PrinterQRcodeSetting", obj);
    return true;
}

bool DPrint::cmd_SetPrinterMode(int mode)
{
    QJsonObject obj;
    obj.insert("mode", mode);

    SendCommand("SetPrinterMode", obj);
    return true;
}

bool DPrint::cmd_SetPrinterQRCodeType(int type)
{
    QJsonObject obj;
    obj.insert("type", type);

    SendCommand("SetPrinterQRCodeType", obj);
    return true;
}

bool DPrint::cmd_SetPrinterBarCodeType(int type)
{
    QJsonObject obj;
    obj.insert("type", type);

    SendCommand("SetPrinterBarCodeType", obj);
    return true;
}




