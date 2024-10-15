#ifndef DEVICE_PROTOCOL_H
#define DEVICE_PROTOCOL_H

#include <QJsonValue>
#include <QByteArray>
#include <QMap>
#include <QVector>

class DeviceProtocol
{
public:
    static QMap<QString, quint16> CmdIdMap;

    static quint16 CmdId(const QString &command);

    //![1] (0xAXXX) Download
    /* 0. System Function (0xA00X) */
    static QByteArray SystemReset(quint32 version, quint16 count);
    static QByteArray SystemTimeSync(quint16 year, quint8 month, quint8 day, quint8 hh, quint8 mm, quint8 sec, quint16 msec);

    /* 1.2 Combo Action Download (0xA01X) */
    static QByteArray ComboActionDownload(QVector<quint8> version, quint16 count);
    static QByteArray ComboActionHead(quint16 func_id, quint16 count);
    static QByteArray ComboActionBody(quint16 index, quint16 func_id, QVector<quint32> params);
    static QByteArray ComboActionEnd(quint16 func_id, quint16 count);
    static QByteArray ComboActionDownloadFinish(QVector<quint8> version, quint16 count);
    static QByteArray ComboActionStart(quint16 func_id, quint8 uid, quint32 timeout, QVector<quint32> params);
    static QJsonValue HandleComboActionStart(const QByteArray &array);
    static QByteArray ComboActionStop(quint16 fid, quint8 uid);
    static QByteArray ComboActionAllStop();

    /* 1.3 IAP Upgrade (0xA03X) */
    static QByteArray IAPUpgrade();
    static QByteArray IAPUpgradeStart(quint32 size);
    static QByteArray IAPUpgradeBody(quint32 index, QVector<quint8> params);
    static QByteArray IAPUpgradeFinish(quint32 size);

    /* 1.4 Print Combo (0xA04X) */
    static QByteArray PrinterReset();
    static QByteArray PrinterHeadReset();
    static QByteArray PrinterContentSetting(quint8 row, quint8 isChinese, const QByteArray &content);
    static QByteArray PrinterChangeRibbon(quint8 change);
    static QByteArray PrinterContractRibbon();
    static QByteArray PrinterBarcodeSetting(const QString &code);
    static QByteArray PrinterQRcodeSetting(const QString &code);

    static QByteArray PrintHeadDown();
    static QByteArray PrintTextData();
    static QByteArray PrintQRCode();
    static QByteArray PrintBarCode();
    static QByteArray PrintHeadUp();

    //![2] (0xBXXX) Setting
    /* 2.2 Motor Config */
    static QByteArray SetMotorSubdivision(quint8 mid, quint16 subdvision);
    static QByteArray SetMotorHoldCurrent(quint8 mid, quint8 holdLevel, quint8 runLevel);
    static QByteArray SetMotorSinglePos(quint8 mid, quint8 posid, quint32 coord);
    static QByteArray SetMotorAllPos(quint8 mid, quint8 posCount, quint32 *coord);
    static QByteArray SetMotorSpeed(quint8 mid, quint16 resetSpeed, quint16 startSpeed, quint16 maxSpeed);
    static QByteArray SetMotorResetOverShoot(quint8 mid, quint16 steps);

    /* 2.4 Internal Flash */
    static QByteArray InternalFlashSave();
    static QByteArray InternalFlashLoad();

    /* 2.5 External Flash */
    static QByteArray ExternalFlashSave();
    static QByteArray ExternalFlashLoad();

    /* 2.7 Temperature */
    static QByteArray SetHeaterTempValue(quint8 id, quint16 target, quint16 threshold);

    /* 2.11 Ultrasonic */
    static QByteArray SetUltrasonicKw(quint8 id, quint16 kw);
    static QByteArray SetUltrasonic(quint8 id, quint8 on);
    static QByteArray SetAllUltrasonicKw(quint8 count, quint16 kw);

    /* 2.12 Printer */
    static QByteArray SetPrinterMode(quint8 mode);
    static QByteArray SetPrinterHeadHeight(quint16 height);
    static QByteArray SetPrinterGrayLevel(quint8 level);
    static QByteArray SetPrinterContractSteps(quint8 steps);
    static QByteArray SetPrinterAngle(quint8 angle);
    static QByteArray SetPrinterQRCodeType(quint8 type);
    static QByteArray SetPrinterBarCodeType(quint8 type);
    static QByteArray SetPrinterWaitHeight(quint16 height);

    //![3] (0xCXXX) Query
    /* 3.1 (0xC01X) Temperature */
    static QByteArray QueryTemperature(quint8 id);
    static QJsonValue HandleQueryTemperature(const QByteArray &array);
    static QByteArray QueryTemperatureWet();
    static QJsonValue HandleQueryTemperatureWetRes(const QByteArray &array);
    static QByteArray QueryTemperatureTarget(quint8 id);
    static QJsonValue HandleQueryTemperatureTarget(const QByteArray &array);
    static QByteArray QueryAllTemperature(quint8 count);
    static QJsonValue HandleQueryAllTemperature(const QByteArray &array);

    /* 3.2 (0xC02X) Pressure */
    static QByteArray QueryPressure(quint8 id);
    static QJsonValue HandleQueryPressureRes(const QByteArray &array);
    static QByteArray QueryPressurePN();
    static QJsonValue HandleQueryPressurePNRes(const QByteArray &array);
    static QByteArray QueryAllPressure(quint8 count);
    static QJsonValue HandleQueryAllPressureRes(const QByteArray &array);

    /* 3.2 (0xC03X) Floater */
    static QByteArray QueryFloater(quint8 id);
    static QJsonValue HandleQueryFloaterRes(const QByteArray &array);
    static QByteArray QueryAllFloater(quint8 count);
    static QJsonValue HandleQueryAllFloaterRes(const QByteArray &array);

    /* 3.4 (0xC04X) Motor pos query */
    static QByteArray QueryMotorSinglePos(quint8 mid, quint8 posid);
    static QJsonValue HandleQueryMotorSinglePosRes(const QByteArray &array);
    static QByteArray QueryMotorAllPos(quint8 mid, quint8 count);
    static QJsonValue HandleQueryMotorAllPosRes(const QByteArray &array);
    static QByteArray QueryMotorResetOverShoot(quint8 mid);
    static QJsonValue HandleQueryMotorResetOverShoot(const QByteArray &array);

    /* 3.5 (0xC05X) Sensor Value */
    static QByteArray GetSensorValue(quint8 sid);
    static QByteArray QuerySensorValue(quint8 sid);
    static QJsonValue HandleQuerySensorValueRes(const QByteArray &array);

    static QByteArray GetAllSensorValue(quint8 count);
    static QByteArray QueryAllSensorValue(quint8 count);
    static QJsonValue HandleQueryAllSensorValueRes(const QByteArray &array);

    static QByteArray QueryComboActionVersion();
    static QJsonValue HandleQueryComboActionVersion(const QByteArray &array);
    static QByteArray QueryMCUSoftVersion();
    static QJsonValue HandleQueryMCUSoftVersion(const QByteArray &array);

    /* 3.17 (0xC12X) Query Printer Setting */
    static QByteArray GetPrinterSensors();
    static QJsonValue handlePrinterSensors(const QByteArray &array);
    static QByteArray GetPrinterHeadTemp();
    static QJsonValue handlePrinterHeadTemp(const QByteArray &array);
    static QByteArray GetPrinterHeadHeight();
    static QJsonValue handlePrinterHeadHeight(const QByteArray &array);
    static QByteArray GetPrinterGrayLevel();
    static QJsonValue handlePrinterGrayLevel(const QByteArray &array);
    static QByteArray GetPrinterContractSteps();
    static QJsonValue handlePrinterContractSteps(const QByteArray &array);
    static QByteArray GetPrinterAngle();
    static QJsonValue handlePrinterAngle(const QByteArray &array);
    static QByteArray GetPrinterWaitHeight();
    static QJsonValue handlePrinterWaitHeight(const QByteArray &array);

    //![4] (0xDXXX) Debug
    /* 4.1 Motor */
    static QByteArray MoveMotorToResetPos(quint8 mid, quint16 speed);
    static QByteArray ResetMotorPos(quint8 mid, quint16 speed);
    static QByteArray MoveForward(quint8 mid, quint16 speed, quint32 steps);
    static QByteArray MoveBackward(quint8 mid, quint16 speed, quint32 steps);
    static QByteArray MoveToCoord(quint8 mid, quint16 speed, quint32 coord);
    static QByteArray MoveToPosid(quint8 mid, quint16 speed, quint8 posid);

    /* 4.2 Pump */
    static QByteArray PumpOnOff(quint8 pid, quint8 on);
    static QByteArray AllPumpOnOff(quint8 on);

    /* 4.3 Valve */
    static QByteArray ValveOnOff(quint8 vid, quint8 on);
    static QByteArray AllValveOnOff(quint8 on);

    /* 4.7 Temperature */
    static QByteArray OpenHeater(quint8 id);
    static QByteArray CloseHeater(quint8 id);
};

#endif // DEVICE_PROTOCOL_H
