#ifndef RT_DEVICE_H
#define RT_DEVICE_H

#include "rt_device_base.h"

class RtDevice : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit RtDevice(const QString &dev_id, quint16 address = 0,
                      const QString &type = QString(), QObject *parent = nullptr);

    /* set command */
    void setComboAction(const QString &function, int funcid, int timeout = 0, const QString &description = QString());

    virtual bool handlePacket(quint16 &can_id, const JPacket &p) override;
    virtual QJsonValue handleResult(const WsRequest &req, const QByteArray &array) override;
    virtual QJsonValue handleSelfResult(const QString &api, const QByteArray &array) override;

    quint16 RunFunction(const QString &api);
    quint16 RunFunctionArg(const QString &api, const QJsonValue &value);
    quint16 RunFunctionArg(const QString &api, const QByteArray &data);
    bool isSelfFuncDone(const QString &api);

//![1] Download (0xAXXX)
    /* 1.0. System Function (0xA00X) */
    QByteArray SystemReset(const QJsonValue &value);
    QByteArray SystemTimeSync(const QJsonValue &value);

    /* 1.2 Combo Action Download (0xA01X) */
    QByteArray ComboActionDownload(const QJsonValue &value);
    QByteArray ComboActionHead(const QJsonValue &value);
    QByteArray ComboActionBody(const QJsonValue &value);
    QByteArray ComboActionEnd(const QJsonValue &value);
    QByteArray ComboActionDownloadFinish(const QJsonValue &value);

    QByteArray ComboActionStart(const QJsonValue &value);
    QByteArray ComboActionStop(const QJsonValue &value);
    QByteArray ComboActionAllStop();

    /* 1.3 IAP Upgrade (0xA03X) */
    QByteArray IAPUpgrade();
    QByteArray IAPUpgradeStart(const QJsonValue &value);
    QByteArray IAPUpgradeBody(const QJsonValue &value);
    QByteArray IAPUpgradeFinish(const QJsonValue &value);

    /* 1.4 Print Download (0xA04X) */
    QByteArray PrinterReset();
    QByteArray PrinterHeadReset();
    QByteArray PrinterContentSetting(const QJsonValue &value);
    QByteArray PrinterChangeRibbon(const QJsonValue &value);
    QByteArray PrinterContractRibbon();
    QByteArray PrinterQRcodeSetting(const QJsonValue &value);
    QByteArray PrinterBarcodeSetting(const QJsonValue &value);

    QByteArray PrintHeadDown();
    QByteArray PrintTextData();
    QByteArray PrintQRCode();
    QByteArray PrintBarCode();
    QByteArray PrintHeadUp();

//![2] Setting (0xBXXX)
    /* 2.2 Motor Config (0xB02X) */
    QByteArray SetMotorSubdivision(const QJsonValue &value);
    QByteArray SetMotorHoldCurrent(const QJsonValue &value);
    QByteArray SetMotorSinglePos(const QJsonValue &value);
    QByteArray SetMotorAllPos(const QJsonValue &value);
    QByteArray SetMotorSpeed(const QJsonValue &value);
    QByteArray SetMotorResetOverShoot(const QJsonValue &value);

    /* 2.4 Internal Flash (0xB04X) */
    QByteArray InternalFlashSave();
    QByteArray InternalFlashLoad();

    /* 2.5 External Flash (0xB05X) */
    QByteArray ExternalFlashSave();
    QByteArray ExternalFlashLoad();

    /* 2.7 Temperature (0xB07X) */
    QByteArray SetHeaterTempValue(const QJsonValue &value);

    /* 2.11 Ultrasonic (0xB0BX) */
    QByteArray SetUltrasonicKw(const QJsonValue &value);
    QByteArray SetUltrasonic(const QJsonValue &value);
    QByteArray SetAllUltrasonicKw(const QJsonValue &value);

    /* 2.12 Printer (0xB0CX) */
    QByteArray SetPrinterMode(const QJsonValue &value);
    QByteArray SetPrinterHeadHeight(const QJsonValue &value);
    QByteArray SetPrinterGrayLevel(const QJsonValue &value);
    QByteArray SetPrinterContractSteps(const QJsonValue &value);
    QByteArray SetPrinterAngle(const QJsonValue &value);
    QByteArray SetPrinterQRCodeType(const QJsonValue &value);
    QByteArray SetPrinterBarCodeType(const QJsonValue &value);
    QByteArray SetPrinterWaitHeight(const QJsonValue &value);

//![3] Query Infomaton (0xCXXX)
    /* 3.1 Temperature (0xC01X) */
    QByteArray QueryTemperature(const QJsonValue &value);
    QByteArray QueryTemperatureWet();
    QByteArray QueryTemperatureTarget(const QJsonValue &value);
    QByteArray QueryAllTemperature(const QJsonValue &value);

    /* 3.2 Pressure (0xC02X) */
    QByteArray QueryPressure(const QJsonValue &value);
    QByteArray QueryPressurePN();
    QByteArray QueryAllPressure(const QJsonValue &value);

    /* 3.3 Floater (0xC03X) */
    QByteArray QueryFloater(const QJsonValue &value);
    QByteArray QueryAllFloater(const QJsonValue &value);

    /* 3.4 Motor pos query (0xC04X) */
    QByteArray QueryMotorSinglePos(const QJsonValue &value);
    QByteArray QueryMotorAllPos(const QJsonValue &value);
    QByteArray QueryMotorResetOverShoot(const QJsonValue &value);

    /* 3.5 Sensor Value (0xC05X) */
    QByteArray GetSensorValue(const QJsonValue &value);
    QByteArray QuerySensorValue(const QJsonValue &value);
    QByteArray GetAllSensorValue(const QJsonValue &value);
    QByteArray QueryAllSensorValue(const QJsonValue &value);

    /* Version */
    QByteArray QueryComboActionVersion();
    QByteArray QueryMCUSoftVersion();

    /* 3.6 Printer  (0xC12X)*/
    QByteArray GetPrinterHeadHeight();
    QByteArray GetPrinterContractSteps();
    QByteArray GetPrinterWaitHeight();
	
//![4] Debug (0xDXXX)
    /* 4.1 Motor Debug (0xD01X) */
    QByteArray MoveMotorToResetPos(const QJsonValue &value);
    QByteArray ResetMotorPos(const QJsonValue &value);
    QByteArray MoveForward(const QJsonValue &value);
    QByteArray MoveBackward(const QJsonValue &value);
    QByteArray MoveToCoord(const QJsonValue &value);
    QByteArray MoveToPosid(const QJsonValue &value);

    /* 4.2 Pump Debug (0xD02X) */
    QByteArray PumpOnOff(const QJsonValue &value);
    QByteArray AllPumpOnOff(const QJsonValue &value);

    /* 4.3 Valve Debug (0xD03X) */
    QByteArray ValveOnOff(const QJsonValue &value);
    QByteArray AllValveOnOff(const QJsonValue &value);

    /* 4.3 Valve Debug (0xD07X) */
    QByteArray OpenHeater(const QJsonValue &value);
    QByteArray CloseHeater(const QJsonValue &value);

private:
    typedef QByteArray (RtDevice::*pFunction)();
    typedef QByteArray (RtDevice::*pFunctionArg)(const QJsonValue &value);

    QMap<QString, pFunction> FunctionMap;
    QMap<QString, pFunctionArg> FunctionArgMap;
    QMap<QString, bool> mSelfSendingMap;
};

#endif // RT_DEVICE_H
