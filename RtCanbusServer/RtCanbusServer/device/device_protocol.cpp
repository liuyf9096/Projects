#include "device_protocol.h"
#include "f_common.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QStringList>

QMap<QString, quint16> DeviceProtocol::CmdIdMap = {
    {"SystemReset",             0xA001},
    {"SystemTimeSync",          0xA002},

    {"ComboActionDownload",     0xA021},
    {"ComboActionHead",         0xA022},
    {"ComboActionBody",         0xA023},
    {"ComboActionEnd",          0xA024},
    {"ComboActionDownloadFinish",0xA025},
    {"ComboActionStart",        0xA026},
    {"ComboActionStop",         0xA027},
    {"ComboActionAllStop",      0xA028},

    {"IAPUpgrade",              0xA030},
    {"IAPUpgradeStart",         0xA031},
    {"IAPUpgradeBody",          0xA032},
    {"IAPUpgradeFinish",        0xA033},

    {"PrinterReset",            0xA040},
    {"PrinterHeadReset",        0xA041},
    {"PrinterContentSetting",   0xA042},
    {"PrinterChangeRibbon",     0xA043},
    {"PrinterContractRibbon",   0xA044},
    {"PrinterQRcodeSetting",    0xA045},
    {"PrinterBarcodeSetting",   0xA046},

    {"PrintHeadDown",           0xA060},
    {"PrintTextData",           0xA061},
    {"PrintQRCode",             0xA062},
    {"PrintBarCode",            0xA063},
    {"PrintHeadUp",             0xA064},

    {"SetMotorSubdivision",     0xB021},
    {"SetMotorHoldCurrent",     0xB022},
    {"SetMotorSinglePos",       0xB023},
    {"SetMotorAllPos",          0xB024},
    {"SetMotorSpeed",           0xB027},
    {"SetMotorResetOverShoot",  0xB02B},

    {"InternalFlashSave",       0xB041},
    {"InternalFlashLoad",       0xB042},
    {"ExternalFlashSave",       0xB051},
    {"ExternalFlashLoad",       0xB052},

    {"SetHeaterTempValue",      0xB072},

    {"SetUltrasonicKw",         0xB0B1},
    {"SetUltrasonic",           0xB0B2},
    {"SetAllUltrasonicKw",      0xB0BA},

    {"SetPrinterMode",          0xB0C1},
    {"SetPrinterHeadHeight",    0xB0C2},
    {"SetPrinterGrayLevel",     0xB0C3},
    {"SetPrinterContractSteps", 0xB0C4},
    {"SetPrinterAngle",         0xB0C5},
    {"SetPrinterQRCodeType",    0xB0C6},
    {"SetPrinterBarCodeType",   0xB0C7},
    {"SetPrinterWaitHeight",    0xB0C8},

    {"QueryTemperature",        0xC011},
    {"QueryTemperatureWet",     0xC012},
    {"QueryAllTemperature",     0xC01A},

    {"QueryPressure",           0xC021},
    {"QueryPressurePN",         0xC022},
    {"QueryAllPressure",        0xC02A},

    {"QueryFloater",            0xC031},
    {"QueryAllFloater",         0xC03A},

    {"QueryMotorSinglePos",     0xC041},
    {"QueryMotorAllPos",        0xC042},
    {"QueryMotorResetOverShoot",0xC044},

    {"GetSensorValue",          0xC051},
    {"QuerySensorValue",        0xC051},
    {"GetAllSensorValue",       0xC05A},
    {"QueryAllSensorValue",     0xC05A},

    {"QueryComboActionVersion", 0xC071},
    {"QueryMCUSoftVersion",     0xC0C8},

    {"GetPrinterSensors",       0xC121},
    {"GetPrinterHeadTemp",      0xC122},
    {"GetPrinterHeadHeight",    0xC123},
    {"GetPrinterGrayLevel",     0xC124},
    {"GetPrinterContractSteps", 0xC125},
    {"GetPrinterAngle",         0xC126},
    {"GetPrinterWaitHeight",    0xC127},

    {"MoveMotorToResetPos",     0xD010},
    {"ResetMotorPos",           0xD011},
    {"MoveForward",             0xD012},
    {"MoveBackward",            0xD013},
    {"MoveToCoord",             0xD014},
    {"MoveToPosid",             0xD015},

    {"PumpOnOff",               0xD021},
    {"AllPumpOnOff",            0xD02A},
    {"ValveOnOff",              0xD031},
    {"AllValveOnOff",           0xD03A},

    {"OpenHeater",              0xD071},
    {"CloseHeater",             0xD072}
};

quint16 DeviceProtocol::CmdId(const QString &command)
{
    return CmdIdMap.value(command);
}

QByteArray DeviceProtocol::SystemReset(quint32 version, quint16 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&version), sizeof (quint32));
    data.append(reinterpret_cast<char*>(&count), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SystemTimeSync(quint16 year, quint8 month, quint8 day, quint8 hh, quint8 mm, quint8 sec, quint16 msec)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&year), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&month), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&day), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&hh), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&mm), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&sec), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&msec), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::ComboActionDownload(QVector<quint8> version, quint16 count)
{
    QByteArray data;
    if (version.count() > 3) {
        for (int i = 0; i < 4; ++i) {
            quint8 v = version.at(i);
            data.append(reinterpret_cast<char*>(&v), sizeof (quint8));
        }
        data.append(reinterpret_cast<char*>(&count), sizeof (quint16));
    }
    return data;
}

QByteArray DeviceProtocol::ComboActionHead(quint16 func_id, quint16 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&func_id), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&count), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::ComboActionBody(quint16 index, quint16 func_id, QVector<quint32> params)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&index), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&func_id), sizeof (quint16));
    for (int i = 0; i < params.count(); ++i) {
        quint32 param = params.at(i);
        data.append(reinterpret_cast<char*>(&param), sizeof (quint32));
    }
    return data;
}

QByteArray DeviceProtocol::ComboActionEnd(quint16 func_id, quint16 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&func_id), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&count), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::ComboActionDownloadFinish(QVector<quint8> version, quint16 count)
{
    QByteArray data;
    if (version.count() > 3) {
        for (int i = 0; i < 4; ++i) {
            quint8 v = version.at(i);
            data.append(reinterpret_cast<char*>(&v), sizeof (quint8));
        }
        data.append(reinterpret_cast<char*>(&count), sizeof (quint16));
    }
    return data;
}

QByteArray DeviceProtocol::ComboActionStart(quint16 func_id, quint8 uid, quint32 timeout, QVector<quint32> params)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&func_id), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&uid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&timeout), sizeof (quint32));

    quint8 count = params.count();
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));

    for (int i = 0; i < count; ++i) {
        quint32 param = params.at(i);
        data.append(reinterpret_cast<char*>(&param), sizeof (quint32));
    }
    return data;
}

QJsonValue DeviceProtocol::HandleComboActionStart(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        QJsonArray arr_8;
        for (int i = 0; i < array.count(); ++i) {
            quint8 a = 0;
            out >> a;
            arr_8.append(a);
        }

        QJsonArray arr_16;
        if (value.count() % 2 == 0) {
            for (int i = 0; i < value.count() / 2; ++i) {
                out.device()->seek(0);
                quint16 a = 0;
                out >> a;
                arr_16.append(a);
            }
        }

        QJsonArray arr_32;
        if (value.count() % 4 == 0) {
            for (int i = 0; i < value.count() / 4; ++i) {
                out.device()->seek(0);
                quint32 a = 0;
                out >> a;
                arr_32.append(static_cast<double>(a));
            }
        }

        QString str;
        str.append(value);

        QJsonObject obj;
        obj.insert("value_8", arr_8);
        obj.insert("value_16", arr_16);
        if (arr_32.isEmpty() == false) {
            obj.insert("value_32", arr_32);
        }
        obj.insert("string", str);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::ComboActionStop(quint16 fid, quint8 uid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&fid), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&uid), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::ComboActionAllStop()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::IAPUpgrade()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::IAPUpgradeStart(quint32 size)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&size), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::IAPUpgradeBody(quint32 index, QVector<quint8> params)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&index), sizeof (quint32));
    for (int i = 0; i < params.count(); ++i) {
        quint8 param = params.at(i);
        data.append(reinterpret_cast<char*>(&param), sizeof (quint8));
    }
    return data;
}

QByteArray DeviceProtocol::IAPUpgradeFinish(quint32 size)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&size), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::PrinterReset()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrinterHeadReset()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrinterContentSetting(quint8 row, quint8 isChinese, const QByteArray &content)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&row), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&isChinese), sizeof (quint8));
    data.append(content);
    return data;
}

QByteArray DeviceProtocol::PrinterChangeRibbon(quint8 change)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&change), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::PrinterContractRibbon()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrinterBarcodeSetting(const QString &code)
{
    QByteArray data;
    quint8 reserve = 0;
    data.append(reinterpret_cast<char*>(&reserve), sizeof (quint8));
    data.append(code);
    return data;
}

QByteArray DeviceProtocol::PrinterQRcodeSetting(const QString &code)
{
    QByteArray data;
    quint8 reserve = 0;
    data.append(reinterpret_cast<char*>(&reserve), sizeof (quint8));
    data.append(code);
    return data;
}

QByteArray DeviceProtocol::PrintHeadDown()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrintTextData()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrintQRCode()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrintBarCode()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::PrintHeadUp()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::SetMotorSubdivision(quint8 mid, quint16 subdvision)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&subdvision), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetMotorHoldCurrent(quint8 mid, quint8 holdLevel, quint8 runLevel)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&holdLevel), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&runLevel), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetMotorSinglePos(quint8 mid, quint8 posid, quint32 coord)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&posid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&coord), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::SetMotorAllPos(quint8 mid, quint8 posCount, quint32 *coord)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&posCount), sizeof (quint8));
    for (int i = 0; i < posCount; ++i) {
        quint32 coord_i = coord[i];
        data.append(reinterpret_cast<char*>(&coord_i), sizeof (quint32));
    }
    return data;
}

QByteArray DeviceProtocol::SetMotorSpeed(quint8 mid, quint16 resetSpeed, quint16 startSpeed, quint16 maxSpeed)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&resetSpeed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&startSpeed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&maxSpeed), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetMotorResetOverShoot(quint8 mid, quint16 steps)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&steps), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::InternalFlashSave()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::InternalFlashLoad()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::ExternalFlashSave()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::ExternalFlashLoad()
{
    QByteArray data;
    return data;
}

QByteArray DeviceProtocol::SetHeaterTempValue(quint8 id, quint16 target, quint16 threshold)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&target), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&threshold), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetUltrasonicKw(quint8 id, quint16 kw)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&kw), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetUltrasonic(quint8 id, quint8 on)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&on), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetAllUltrasonicKw(quint8 count, quint16 kw)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&kw), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetPrinterMode(quint8 mode)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mode), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterHeadHeight(quint16 height)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&height), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::SetPrinterGrayLevel(quint8 level)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&level), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterContractSteps(quint8 steps)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&steps), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterAngle(quint8 angle)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&angle), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterQRCodeType(quint8 type)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&type), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterBarCodeType(quint8 type)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&type), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::SetPrinterWaitHeight(quint16 height)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&height), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::QueryTemperature(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::QueryTemperatureWet()
{
    QByteArray data;
    return data;
}

QJsonValue DeviceProtocol::HandleQueryTemperatureWetRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 temp = 0;
        quint8 humidity = 0;
        out >> temp >> humidity;

        QJsonObject obj;
        obj.insert("temp", temp);
        obj.insert("humidity", humidity);
        return obj;
    }
    return true;
}

QJsonValue DeviceProtocol::HandleQueryTemperature(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 3) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 id = 0;
        quint16 temp = 0;
        out >> id >> temp;

        QJsonObject obj;
        obj.insert("id", id);
        obj.insert("temp", temp);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryTemperatureTarget(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryTemperatureTarget(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 10) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 id = 0;
        quint16 target = 0, threshold = 0, tP = 0, tI = 0, tD = 0;

        out >> id >> target >> threshold >> tP >> tI >> tD;

        QJsonObject obj;
        obj.insert("id", id);
        obj.insert("target", target);
        obj.insert("threshold", threshold);
        obj.insert("tP", tP);
        obj.insert("tI", tI);
        obj.insert("tD", tD);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryAllTemperature(quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryAllTemperature(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 3) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 count = 0;
        out >> count;

        QJsonArray arr;
        for (int i = 0; i < count; ++i) {
            quint16 temp = 0;
            out >> temp;
            arr.append(temp);
        }

        QJsonObject obj;
        obj.insert("count", count);
        obj.insert("temps", arr);

        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryPressure(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::QueryPressurePN()
{
    QByteArray data;
    return data;
}

QJsonValue DeviceProtocol::HandleQueryPressurePNRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 positive = 0;
        quint8 negative = 0;
        out >> positive >> negative;

        QJsonObject obj;
        obj.insert("positive", positive);
        obj.insert("negative", negative);
        return obj;
    }
    return true;
}

QJsonValue DeviceProtocol::HandleQueryPressureRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 3) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 id = 0;
        quint16 pressure = 0;
        out >> id >> pressure;

        QJsonObject obj;
        obj.insert("id", id);
        obj.insert("value", pressure);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryAllPressure(quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryAllPressureRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 3) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 count = 0;
        out >> count;

        QJsonArray arr;
        for (int i = 0; i < count; ++i) {
            quint16 pressure = 0;
            out >> pressure;
            arr.append(pressure);
        }

        QJsonObject obj;
        obj.insert("count", count);
        obj.insert("values", arr);

        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryFloater(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryFloaterRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 id = 0;
        quint8 status = 0;
        out >> id >> status;

        QJsonObject obj;
        obj.insert("id", id);
        obj.insert("value", status);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryAllFloater(quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryAllFloaterRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 count = 0;
        out >> count;

        QJsonArray arr;
        for (int i = 0; i < count; ++i) {
            quint8 status = 0;
            out >> status;
            arr.append(status);
        }

        QJsonObject obj;
        obj.insert("count", count);
        obj.insert("values", arr);

        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryMotorSinglePos(quint8 mid, quint8 posid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&posid), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryMotorSinglePosRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 6) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 mid = 0, posid = 0;
        quint32 coord = 0;
        out >> mid >> posid >> coord;

        QJsonObject obj;
        obj.insert("mid", mid);
        obj.insert("posid", posid);
        obj.insert("coord", static_cast<double>(coord));
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryMotorAllPos(quint8 mid, quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryMotorAllPosRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 mid = 0, count = 0;
        out >> mid >> count;

        QJsonArray arr;
        for (int i = 0; i < count; ++i) {
            quint32 coord = 0;
            out >> coord;
            arr.append(static_cast<double>(coord));
        }

        QJsonObject obj;
        obj.insert("mid", mid);
        obj.insert("count", count);
        obj.insert("coords", arr);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryMotorResetOverShoot(quint8 mid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryMotorResetOverShoot(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 3) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 mid = 0;
        quint16 steps = 0;
        out >> mid >> steps;

        QJsonObject obj;
        obj.insert("mid", mid);
        obj.insert("steps", steps);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetSensorValue(quint8 sid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&sid), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::QuerySensorValue(quint8 sid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&sid), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQuerySensorValueRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 sid = 0, svalue = 0;
        out >> sid >> svalue;

        QJsonObject obj;
        obj.insert("sid", sid);
        obj.insert("value", svalue);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetAllSensorValue(quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::QueryAllSensorValue(quint8 count)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&count), sizeof (quint8));
    return data;
}

QJsonValue DeviceProtocol::HandleQueryAllSensorValueRes(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 1) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 count = 0;
        out >> count;

        QByteArray sensorValue = array.mid(1);
        QBitArray bits = FCommon::bytesToBits(sensorValue);
        QJsonObject valueObj;
        for (int i = 0; i < bits.count(); ++i) {
            valueObj.insert(QString::number(i), bits.at(i));
        }

        QJsonObject obj;
        obj.insert("count", count);
        obj.insert("values", valueObj);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryComboActionVersion()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::HandleQueryComboActionVersion(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 10) {
        QString version = value;
        QJsonObject obj;
        obj.insert("version", version);

        if (version.startsWith("V")) {
            version.remove(0, 1);
            QStringList list = version.split(".");
            if (list.count() > 2) {
                QString major = list.at(0);
                QString minor = list.at(1);
                QString patch = list.at(2);
                obj.insert("major", major.toInt());
                obj.insert("minor", minor.toInt());
                obj.insert("patch", patch.toInt());
            }
        }
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::QueryMCUSoftVersion()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::HandleQueryMCUSoftVersion(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 10) {
        QString version = value;
        QJsonObject obj;
        obj.insert("version", version);

        if (version.startsWith("V")) {
            version.remove(0, 1);
            QStringList list = version.split(".");
            if (list.count() > 2) {
                QString major = list.at(0);
                QString minor = list.at(1);
                QString patch = list.at(2);
                obj.insert("major", major.toInt());
                obj.insert("minor", minor.toInt());
                obj.insert("patch", patch.toInt());
            }
        }
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterSensors()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterSensors(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() >= 2) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 sid = 0, svalue = 0;
        out >> sid >> svalue;

        QJsonObject obj;
        obj.insert("sid", sid);
        obj.insert("value", svalue);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterHeadTemp()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterHeadTemp(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 temp = 0;
        out >> temp;

        QJsonObject obj;
        obj.insert("temp", temp);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterHeadHeight()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterHeadHeight(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint16 height = 0;
        out >> height;

        QJsonObject obj;
        obj.insert("height", height);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterGrayLevel()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterGrayLevel(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 level = 0;
        out >> level;

        QJsonObject obj;
        obj.insert("level", level);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterContractSteps()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterContractSteps(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 steps = 0;
        out >> steps;

        QJsonObject obj;
        obj.insert("steps", steps);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterAngle()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterAngle(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 angle = 0;
        out >> angle;

        QJsonObject obj;
        obj.insert("angle", angle);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::GetPrinterWaitHeight()
{
    return QByteArray();
}

QJsonValue DeviceProtocol::handlePrinterWaitHeight(const QByteArray &array)
{
    QByteArray value = array;
    if (value.count() > 0) {
        QDataStream out(&value, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint16 height = 0;
        out >> height;

        QJsonObject obj;
        obj.insert("height", height);
        return obj;
    }
    return true;
}

QByteArray DeviceProtocol::MoveMotorToResetPos(quint8 mid, quint16 speed)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::ResetMotorPos(quint8 mid, quint16 speed)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    return data;
}

QByteArray DeviceProtocol::MoveForward(quint8 mid, quint16 speed, quint32 steps)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&steps), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::MoveBackward(quint8 mid, quint16 speed, quint32 steps)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&steps), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::MoveToCoord(quint8 mid, quint16 speed, quint32 coord)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&coord), sizeof (quint32));
    return data;
}

QByteArray DeviceProtocol::MoveToPosid(quint8 mid, quint16 speed, quint8 posid)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&mid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&speed), sizeof (quint16));
    data.append(reinterpret_cast<char*>(&posid), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::PumpOnOff(quint8 pid, quint8 on)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&pid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&on), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::AllPumpOnOff(quint8 on)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&on), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::ValveOnOff(quint8 vid, quint8 on)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&vid), sizeof (quint8));
    data.append(reinterpret_cast<char*>(&on), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::AllValveOnOff(quint8 on)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&on), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::OpenHeater(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

QByteArray DeviceProtocol::CloseHeater(quint8 id)
{
    QByteArray data;
    data.append(reinterpret_cast<char*>(&id), sizeof (quint8));
    return data;
}

