#include "rt_device.h"
#include "device_protocol.h"
#include "messagecenter/f_jsonrpc_parser.h"

#include <QTextCodec>
#include <QDebug>

RtDevice::RtDevice(const QString &dev_id, quint16 address, const QString &type, QObject *parent)
    : RtDeviceBase{dev_id, address, type, parent}
{
    FunctionArgMap.insert("SystemReset", &RtDevice::SystemReset);
    FunctionArgMap.insert("SystemTimeSync", &RtDevice::SystemTimeSync);

    FunctionArgMap.insert("ComboActionStart", &RtDevice::ComboActionStart);
    FunctionArgMap.insert("ComboActionStop", &RtDevice::ComboActionStop);
    FunctionMap.insert("ComboActionAllStop", &RtDevice::ComboActionAllStop);

    FunctionMap.insert("PrinterReset", &RtDevice::PrinterReset);
    FunctionMap.insert("PrinterHeadReset", &RtDevice::PrinterHeadReset);
    FunctionArgMap.insert("PrinterContentSetting", &RtDevice::PrinterContentSetting);
    FunctionArgMap.insert("PrinterChangeRibbon", &RtDevice::PrinterChangeRibbon);
    FunctionMap.insert("PrinterContractRibbon", &RtDevice::PrinterContractRibbon);
    FunctionArgMap.insert("PrinterBarcodeSetting", &RtDevice::PrinterBarcodeSetting);
    FunctionArgMap.insert("PrinterQRcodeSetting", &RtDevice::PrinterQRcodeSetting);

    FunctionMap.insert("PrintHeadDown", &RtDevice::PrintHeadDown);
    FunctionMap.insert("PrintTextData", &RtDevice::PrintTextData);
    FunctionMap.insert("PrintQRCode", &RtDevice::PrintQRCode);
    FunctionMap.insert("PrintBarCode", &RtDevice::PrintBarCode);
    FunctionMap.insert("PrintHeadUp", &RtDevice::PrintHeadUp);

    FunctionArgMap.insert("SetMotorSubdivision", &RtDevice::SetMotorSubdivision);
    FunctionArgMap.insert("SetMotorHoldCurrent", &RtDevice::SetMotorHoldCurrent);
    FunctionArgMap.insert("SetMotorSinglePos", &RtDevice::SetMotorSinglePos);
    FunctionArgMap.insert("SetMotorAllPos", &RtDevice::SetMotorAllPos);
    FunctionArgMap.insert("SetMotorSpeed", &RtDevice::SetMotorSpeed);
    FunctionArgMap.insert("SetMotorResetOverShoot", &RtDevice::SetMotorResetOverShoot);

    FunctionMap.insert("InternalFlashSave", &RtDevice::InternalFlashSave);
    FunctionMap.insert("InternalFlashLoad", &RtDevice::InternalFlashLoad);
    FunctionMap.insert("ExternalFlashSave", &RtDevice::ExternalFlashSave);
    FunctionMap.insert("ExternalFlashLoad", &RtDevice::ExternalFlashLoad);

    FunctionArgMap.insert("SetHeaterTempValue", &RtDevice::SetHeaterTempValue);

    FunctionArgMap.insert("SetUltrasonicKw", &RtDevice::SetUltrasonicKw);
    FunctionArgMap.insert("SetUltrasonic", &RtDevice::SetUltrasonic);
    FunctionArgMap.insert("SetAllUltrasonicKw", &RtDevice::SetAllUltrasonicKw);

    FunctionArgMap.insert("SetPrinterMode", &RtDevice::SetPrinterMode);
    FunctionArgMap.insert("SetPrinterHeadHeight", &RtDevice::SetPrinterHeadHeight);
    FunctionArgMap.insert("SetPrinterGrayLevel", &RtDevice::SetPrinterGrayLevel);
    FunctionArgMap.insert("SetPrinterContractSteps", &RtDevice::SetPrinterContractSteps);
    FunctionArgMap.insert("SetPrinterAngle", &RtDevice::SetPrinterAngle);
    FunctionArgMap.insert("SetPrinterWaitHeight", &RtDevice::SetPrinterWaitHeight);

    FunctionArgMap.insert("QueryTemperature", &RtDevice::QueryTemperature);
    FunctionMap.insert("QueryTemperatureWet", &RtDevice::QueryTemperatureWet);
    FunctionArgMap.insert("QueryTemperatureTarget", &RtDevice::QueryTemperatureTarget);
    FunctionArgMap.insert("QueryAllTemperature", &RtDevice::QueryAllTemperature);

    FunctionArgMap.insert("QueryPressure", &RtDevice::QueryPressure);
    FunctionMap.insert("QueryPressurePN", &RtDevice::QueryPressurePN);
    FunctionArgMap.insert("QueryAllPressure", &RtDevice::QueryAllPressure);

    FunctionArgMap.insert("QueryFloater", &RtDevice::QueryFloater);
    FunctionArgMap.insert("QueryAllFloater", &RtDevice::QueryAllFloater);

    FunctionArgMap.insert("QueryMotorSinglePos", &RtDevice::QueryMotorSinglePos);
    FunctionArgMap.insert("QueryMotorAllPos", &RtDevice::QueryMotorAllPos);
    FunctionArgMap.insert("QueryMotorResetOverShoot", &RtDevice::QueryMotorResetOverShoot);

    FunctionArgMap.insert("GetSensorValue", &RtDevice::GetSensorValue);
    FunctionArgMap.insert("QuerySensorValue", &RtDevice::QuerySensorValue);
    FunctionArgMap.insert("GetAllSensorValue", &RtDevice::GetAllSensorValue);
    FunctionArgMap.insert("QueryAllSensorValue", &RtDevice::QueryAllSensorValue);

    FunctionMap.insert("QueryComboActionVersion", &RtDevice::QueryComboActionVersion);
    FunctionMap.insert("QueryMCUSoftVersion", &RtDevice::QueryMCUSoftVersion);

    FunctionMap.insert("GetPrinterHeadHeight", &RtDevice::GetPrinterHeadHeight);
    FunctionMap.insert("GetPrinterContractSteps", &RtDevice::GetPrinterContractSteps);
    FunctionMap.insert("GetPrinterWaitHeight", &RtDevice::GetPrinterWaitHeight);

    FunctionArgMap.insert("MoveMotorToResetPos", &RtDevice::MoveMotorToResetPos);
    FunctionArgMap.insert("ResetMotorPos", &RtDevice::ResetMotorPos);
    FunctionArgMap.insert("MoveForward", &RtDevice::MoveForward);
    FunctionArgMap.insert("MoveBackward", &RtDevice::MoveBackward);
    FunctionArgMap.insert("MoveToCoord", &RtDevice::MoveToCoord);
    FunctionArgMap.insert("MoveToPosid", &RtDevice::MoveToPosid);

    FunctionArgMap.insert("PumpOnOff", &RtDevice::PumpOnOff);
    FunctionArgMap.insert("AllPumpOnOff", &RtDevice::AllPumpOnOff);

    FunctionArgMap.insert("ValveOnOff", &RtDevice::ValveOnOff);
    FunctionArgMap.insert("AllValveOnOff", &RtDevice::AllValveOnOff);

    FunctionArgMap.insert("OpenHeater", &RtDevice::OpenHeater);
    FunctionArgMap.insert("CloseHeater", &RtDevice::CloseHeater);
}

void RtDevice::setComboAction(const QString &function, int funcid, int timeout, const QString &description)
{
    mComboActionMap.insert(function, funcid);

    QString str = QString("ComboAction:%1, fid:%2 (0x%3)").arg(function).arg(funcid).arg(funcid, 2, 16, QChar('0'));
    if (timeout > 0) {
        qDebug().noquote() << QString("%1, timeout:%2, %3").arg(str).arg(timeout).arg(description);
    } else {
        qDebug().noquote() << QString("%1, %2").arg(str, description);
    }
}

bool RtDevice::handlePacket(quint16 &can_id, const JPacket &p)
{
    QString comboFuncid;

    if (p.api.startsWith("ComboAction")) {
        QJsonObject obj = p.paramsValue.toObject();
        if (obj.contains("api")) {
            comboFuncid = obj.value("api").toString();
            if (mComboActionMap.contains(comboFuncid) == false) {
                qWarning() << deviceID() << "miss ComboAction function api:" << p.api << comboFuncid;
            }
        }
    }

    if (FunctionMap.contains(p.api)) {
        can_id = RunFunction(p.api);
        return true;
    } else if (FunctionArgMap.contains(p.api)) {
        can_id = RunFunctionArg(p.api, p.paramsValue);
        return true;
    }
    qWarning() << deviceID() << "can NOT handle api:" << p.api << comboFuncid;
    return false;
}

QJsonValue RtDevice::handleResult(const WsRequest &req, const QByteArray &array)
{
    if (req.packet.api == "QueryTemperature") {
        return DeviceProtocol::HandleQueryTemperature(array);
    } else if (req.packet.api == "QueryTemperatureWet") {
        return DeviceProtocol::HandleQueryTemperatureWetRes(array);
    } else if (req.packet.api == "QueryTemperatureTarget") {
        return DeviceProtocol::HandleQueryTemperatureTarget(array);
    } else if (req.packet.api == "QueryAllTemperature") {
        return DeviceProtocol::HandleQueryAllTemperature(array);
    } else if (req.packet.api == "QueryPressure") {
        return DeviceProtocol::HandleQueryPressureRes(array);
    } else if (req.packet.api == "QueryPressurePN") {
        return DeviceProtocol::HandleQueryPressurePNRes(array);
    } else if (req.packet.api == "QueryAllPressure") {
        return DeviceProtocol::HandleQueryAllPressureRes(array);
    } else if (req.packet.api == "QueryFloater") {
        return DeviceProtocol::HandleQueryFloaterRes(array);
    } else if (req.packet.api == "QueryAllFloater") {
        return DeviceProtocol::HandleQueryAllFloaterRes(array);
    } else if (req.packet.api == "QueryMotorSinglePos") {
        return DeviceProtocol::HandleQueryMotorSinglePosRes(array);
    } else if (req.packet.api == "QueryMotorAllPos") {
        return DeviceProtocol::HandleQueryMotorAllPosRes(array);
    } else if (req.packet.api == "QueryMotorResetOverShoot") {
        return DeviceProtocol::HandleQueryMotorResetOverShoot(array);
    } else if (req.packet.api == "QuerySensorValue") {
        return DeviceProtocol::HandleQuerySensorValueRes(array);
    } else if (req.packet.api == "QueryAllSensorValue") {
        return DeviceProtocol::HandleQueryAllSensorValueRes(array);
    } else if (req.packet.api == "QueryComboActionVersion") {
        return DeviceProtocol::HandleQueryComboActionVersion(array);
    } else if (req.packet.api == "QueryMCUSoftVersion") {
        return DeviceProtocol::HandleQueryMCUSoftVersion(array);
    } else if (req.packet.api == "GetPrinterSensors") {
        return DeviceProtocol::handlePrinterSensors(array);
    } else if (req.packet.api == "GetPrinterHeadTemp") {
        return DeviceProtocol::handlePrinterHeadTemp(array);
    } else if (req.packet.api == "GetPrinterHeadHeight") {
        return DeviceProtocol::handlePrinterHeadHeight(array);
    } else if (req.packet.api == "GetPrinterGrayLevel") {
        return DeviceProtocol::handlePrinterGrayLevel(array);
    } else if (req.packet.api == "GetPrinterContractSteps") {
        return DeviceProtocol::handlePrinterContractSteps(array);
    } else if (req.packet.api == "GetPrinterAngle") {
        return DeviceProtocol::handlePrinterAngle(array);
    } else if (req.packet.api == "GetPrinterWaitHeight") {
        return DeviceProtocol::handlePrinterWaitHeight(array);
    } else if (req.packet.api == "ComboActionStart") {
        return DeviceProtocol::HandleComboActionStart(array);
    } else {
        qDebug() << "can NOT handle can result value." << req.packet.api << array.toHex(' ');
    }
    return true;
}

QJsonValue RtDevice::handleSelfResult(const QString &api, const QByteArray &array)
{
    Q_UNUSED(array)

    if (mSelfSendingMap.contains(api)) {
        mSelfSendingMap.insert(api, true);
    }
    return true;
}

/* !!!!!! */
quint16 RtDevice::RunFunction(const QString &api)
{
    auto fn = FunctionMap.value(api);
    QByteArray data = (this->*fn)();

    quint16 cmd_id = DeviceProtocol::CmdId(api);
    if (cmd_id > 0) {
        quint16 canid = newId();
        CanReqPacket p(canid, api, cmd_id, data);
        m_sendingList.append(p);
        return canid;
    }
    return 0;
}

quint16 RtDevice::RunFunctionArg(const QString &api, const QJsonValue &value)
{
    QString subapi;
    if (api.startsWith("ComboAction")) {
        QJsonObject obj = value.toObject();
        subapi = obj.value("api").toString();
    }

    auto fn = FunctionArgMap.value(api);
    QByteArray data = (this->*fn)(value);

    quint16 cmd_id = DeviceProtocol::CmdId(api);
    if (cmd_id > 0) {
        quint16 canid = newId();
        CanReqPacket p(canid, api, cmd_id, data);
        p.subapi = subapi;
        m_sendingList.append(p);
        return canid;
    }
    return 0;
}

quint16 RtDevice::RunFunctionArg(const QString &api, const QByteArray &data)
{
    quint16 cmd_id = DeviceProtocol::CmdId(api);
    if (cmd_id > 0) {
        mSelfSendingMap.insert(api, false);
        quint16 canid = newId();
#ifdef Q_OS_LINUX
        CanReqPacket p(canid, api, cmd_id, data);
        m_sendingList.append(p);
#else
        mSelfSendingMap.insert(api, true);
        qDebug() << "Virtual Send:" << data.toHex();
#endif
        return canid;
    }
    return 0;
}

bool RtDevice::isSelfFuncDone(const QString &api)
{
    if (mSelfSendingMap.contains(api)) {
        return mSelfSendingMap.value(api);
    }
    return false;
}

QByteArray RtDevice::SystemReset(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint32 version = static_cast<quint32>(obj.value("version").toDouble());
    quint16 count = obj.value("count").toInt();

    return DeviceProtocol::SystemReset(version, count);
}

QByteArray RtDevice::SystemTimeSync(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 year = obj.value("year").toInt();
    quint8 month = obj.value("month").toInt();
    quint8 day = obj.value("day").toInt();
    quint8 hh = obj.value("hh").toInt();
    quint8 mm = obj.value("mm").toInt();
    quint8 sec = obj.value("sec").toInt();
    quint16 msec = obj.value("msec").toInt();

    return DeviceProtocol::SystemTimeSync(year, month, day, hh, mm, sec, msec);
}

QByteArray RtDevice::ComboActionDownload(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    QString version = obj.value("version").toString();
    QStringList list = version.split(".");
    QVector<quint8> v_arr;
    if (list.count() > 3) {
        for (int i = 0; i < 4; ++i) {
            QString v = list.at(i);
            quint8 v_i = v.toUInt();
            v_arr.append(v_i);
        }
    }
    quint16 count = obj.value("count").toInt();
    return DeviceProtocol::ComboActionDownload(v_arr, count);
}

QByteArray RtDevice::ComboActionHead(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 func_id = obj.value("func_id").toInt();
    quint16 count = obj.value("count").toInt();

    return DeviceProtocol::ComboActionHead(func_id, count);
}

QByteArray RtDevice::ComboActionBody(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 index = obj.value("index").toInt();
    quint16 func_id = obj.value("func_id").toInt();
    QJsonArray argArr = obj.value("arg").toArray();

    QVector<quint32> params;
    for (int i = 0; i < argArr.count(); ++i) {
        params.append(static_cast<quint32>(argArr.at(i).toDouble()));
    }

    return DeviceProtocol::ComboActionBody(index, func_id, params);
}

QByteArray RtDevice::ComboActionEnd(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 func_id = obj.value("func_id").toInt();
    quint16 count = obj.value("count").toInt();

    return DeviceProtocol::ComboActionEnd(func_id, count);
}

QByteArray RtDevice::ComboActionDownloadFinish(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    QString version = obj.value("version").toString();
    QStringList list = version.split(".");
    QVector<quint8> v_arr;
    if (list.count() > 3) {
        for (int i = 0; i < 4; ++i) {
            QString v = list.at(i);
            quint8 v_i = v.toUInt();
            v_arr.append(v_i);
        }
    }
    quint16 count = obj.value("count").toInt();

    return DeviceProtocol::ComboActionDownloadFinish(v_arr, count);
}

QByteArray RtDevice::ComboActionStart(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 func_id = 0;
    if (obj.contains("func_id")) {
        func_id = obj.value("func_id").toInt();
    } else if (obj.contains("api")) {
        QString api = obj.value("api").toString();
        func_id = mComboActionMap.value(api);
    }
    quint8 uid = obj.value("uid").toInt(0);
    quint32 timeout = static_cast<quint32>(obj.value("timeout").toDouble());
    QJsonArray argArr = obj.value("arg").toArray();

    QVector<quint32> params;
    for (int i = 0; i < argArr.count(); ++i) {
        params.append(static_cast<quint32>(argArr.at(i).toDouble()));
    }

    return DeviceProtocol::ComboActionStart(func_id, uid, timeout, params);
}

QByteArray RtDevice::ComboActionStop(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    QString api = obj.value("api").toString();
    quint16 fid = mComboActionMap.value(api);
    quint8 uid = obj.value("uid").toInt(0);

    return DeviceProtocol::ComboActionStop(fid, uid);
}

QByteArray RtDevice::ComboActionAllStop()
{
    return DeviceProtocol::ComboActionAllStop();
}

QByteArray RtDevice::IAPUpgrade()
{
    return DeviceProtocol::IAPUpgrade();
}

QByteArray RtDevice::IAPUpgradeStart(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint32 size = static_cast<quint32>(obj.value("size").toDouble());

    return DeviceProtocol::IAPUpgradeStart(size);
}

QByteArray RtDevice::IAPUpgradeBody(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint32 index = static_cast<quint32>(obj.value("index").toDouble());
    QJsonArray dataArr = obj.value("data").toArray();

    QVector<quint8> data;
    for (int i = 0; i < dataArr.count(); ++i) {
        data.append(static_cast<quint8>(dataArr.at(i).toInt()));
    }

    return DeviceProtocol::IAPUpgradeBody(index, data);
}

QByteArray RtDevice::IAPUpgradeFinish(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint32 size = static_cast<quint32>(obj.value("size").toDouble());

    return DeviceProtocol::IAPUpgradeFinish(size);
}

QByteArray RtDevice::PrinterReset()
{
    return DeviceProtocol::PrinterReset();
}

QByteArray RtDevice::PrinterHeadReset()
{
    return DeviceProtocol::PrinterHeadReset();
}

QByteArray RtDevice::PrinterContentSetting(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 row = obj.value("row").toInt();
    if (row > 3) {
        qDebug() << __FUNCTION__ << "Error: row > 3";
    }

    QString content = obj.value("content").toString();
    QByteArray text;
    quint8 isChinese = 0;
    if (content.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
        /* Chinese */
        isChinese = 1;
        text = QTextCodec::codecForName("GB2312")->fromUnicode(content);
    } else {
        isChinese = 0;
        text = content.toUtf8();
    }

    return DeviceProtocol::PrinterContentSetting(row, isChinese, text);
}

QByteArray RtDevice::PrinterChangeRibbon(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 change = obj.value("change").toInt();

    return DeviceProtocol::GetAllSensorValue(change);
}

QByteArray RtDevice::PrinterContractRibbon()
{
    return DeviceProtocol::PrinterContractRibbon();
}

QByteArray RtDevice::PrinterQRcodeSetting(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    QString code = obj.value("code").toString();
    return DeviceProtocol::PrinterQRcodeSetting(code);
}

QByteArray RtDevice::PrinterBarcodeSetting(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    QString code = obj.value("code").toString();
    return DeviceProtocol::PrinterBarcodeSetting(code);
}

QByteArray RtDevice::PrintHeadDown()
{
    return DeviceProtocol::PrintHeadDown();
}

QByteArray RtDevice::PrintTextData()
{
    return DeviceProtocol::PrintTextData();
}

QByteArray RtDevice::PrintQRCode()
{
    return DeviceProtocol::PrintQRCode();
}

QByteArray RtDevice::PrintBarCode()
{
    return DeviceProtocol::PrintBarCode();
}

QByteArray RtDevice::PrintHeadUp()
{
    return DeviceProtocol::PrintHeadUp();
}

QByteArray RtDevice::SetMotorSubdivision(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 subdvision = obj.value("subdvision").toInt();

    return DeviceProtocol::SetMotorSubdivision(mid, subdvision);
}

QByteArray RtDevice::SetMotorHoldCurrent(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint8 holdLevel = obj.value("holdLevel").toInt();
    quint8 runLevel = obj.value("runLevel").toInt();

    return DeviceProtocol::SetMotorHoldCurrent(mid, holdLevel, runLevel);
}

QByteArray RtDevice::SetMotorSinglePos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint8 posid = obj.value("posid").toInt();
    quint32 coord = static_cast<quint32>(obj.value("coord").toDouble());

    return DeviceProtocol::SetMotorSinglePos(mid, posid, coord);
}

QByteArray RtDevice::SetMotorAllPos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    QJsonArray coordArr = obj.value("coord").toArray();
    quint8 count = coordArr.count();
    quint32* const coord = (quint32*)malloc(sizeof(quint32)*count);
    for (int i = 0; i < count; ++i) {
        coord[i] = coordArr.at(i).toInt();
    }

    return DeviceProtocol::SetMotorAllPos(mid, count, coord);
}

QByteArray RtDevice::SetMotorSpeed(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 resetSpeed = obj.value("resetSpeed").toInt();
    quint16 startSpeed = obj.value("startSpeed").toInt();
    quint16 maxSpeed = obj.value("maxSpeed").toInt();

    return DeviceProtocol::SetMotorSpeed(mid, resetSpeed, startSpeed, maxSpeed);
}

QByteArray RtDevice::SetMotorResetOverShoot(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 steps = obj.value("steps").toInt();

    return DeviceProtocol::SetMotorResetOverShoot(mid, steps);
}

QByteArray RtDevice::InternalFlashSave()
{
    return DeviceProtocol::InternalFlashSave();
}

QByteArray RtDevice::InternalFlashLoad()
{
    return DeviceProtocol::InternalFlashLoad();
}

QByteArray RtDevice::ExternalFlashSave()
{
    return DeviceProtocol::ExternalFlashSave();
}

QByteArray RtDevice::ExternalFlashLoad()
{
    return DeviceProtocol::ExternalFlashLoad();
}

QByteArray RtDevice::SetHeaterTempValue(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();
    quint16 target = obj.value("target").toInt();
    quint16 threshold = obj.value("threshold").toInt(0);

    return DeviceProtocol::SetHeaterTempValue(id, target, threshold);
}

QByteArray RtDevice::SetUltrasonicKw(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();
    quint16 kw = obj.value("kw").toInt();

    return DeviceProtocol::SetUltrasonicKw(id, kw);
}

QByteArray RtDevice::SetUltrasonic(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("channel").toInt();
    quint8 on = obj.value("on").toInt();

    return DeviceProtocol::SetUltrasonic(id, on);
}

QByteArray RtDevice::SetAllUltrasonicKw(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();
    quint16 kw = obj.value("kw").toInt();

    return DeviceProtocol::SetAllUltrasonicKw(count, kw);
}

QByteArray RtDevice::SetPrinterMode(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mode = obj.value("mode").toInt();

    return DeviceProtocol::SetPrinterMode(mode);
}

QByteArray RtDevice::SetPrinterHeadHeight(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 pos = obj.value("height").toInt();

    return DeviceProtocol::SetPrinterHeadHeight(pos);
}

QByteArray RtDevice::SetPrinterGrayLevel(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 level = obj.value("level").toInt();

    return DeviceProtocol::SetPrinterGrayLevel(level);
}

QByteArray RtDevice::SetPrinterContractSteps(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 step = obj.value("steps").toInt();

    return DeviceProtocol::SetPrinterContractSteps(step);
}

QByteArray RtDevice::SetPrinterAngle(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 angle = obj.value("angle").toInt();

    return DeviceProtocol::SetPrinterAngle(angle);
}

QByteArray RtDevice::SetPrinterQRCodeType(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 type = obj.value("type").toInt();

    return DeviceProtocol::SetPrinterQRCodeType(type);
}

QByteArray RtDevice::SetPrinterBarCodeType(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 type = obj.value("type").toInt();

    return DeviceProtocol::SetPrinterBarCodeType(type);
}

QByteArray RtDevice::SetPrinterWaitHeight(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint16 pos = obj.value("height").toInt();

    return DeviceProtocol::SetPrinterWaitHeight(pos);
}

QByteArray RtDevice::QueryTemperature(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::QueryTemperature(id);
}

QByteArray RtDevice::QueryTemperatureWet()
{
    return DeviceProtocol::QueryTemperatureWet();
}

QByteArray RtDevice::QueryTemperatureTarget(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::QueryTemperatureTarget(id);
}

QByteArray RtDevice::QueryAllTemperature(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::QueryAllTemperature(count);
}

QByteArray RtDevice::QueryPressure(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::QueryPressure(id);
}

QByteArray RtDevice::QueryPressurePN()
{
    return DeviceProtocol::QueryPressurePN();
}

QByteArray RtDevice::QueryAllPressure(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::QueryAllPressure(count);
}

QByteArray RtDevice::QueryFloater(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::QueryFloater(id);
}

QByteArray RtDevice::QueryAllFloater(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::QueryAllFloater(count);
}

QByteArray RtDevice::QueryMotorSinglePos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint8 posid = obj.value("posid").toInt();

    return DeviceProtocol::QueryMotorSinglePos(mid, posid);
}

QByteArray RtDevice::QueryMotorAllPos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::QueryMotorSinglePos(mid, count);
}

QByteArray RtDevice::QueryMotorResetOverShoot(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();

    return DeviceProtocol::QueryMotorResetOverShoot(mid);
}

QByteArray RtDevice::GetSensorValue(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 sid = obj.value("sid").toInt();

    return DeviceProtocol::GetSensorValue(sid);
}

QByteArray RtDevice::QuerySensorValue(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 sid = obj.value("sid").toInt();

    return DeviceProtocol::QuerySensorValue(sid);
}

QByteArray RtDevice::GetAllSensorValue(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::GetAllSensorValue(count);
}

QByteArray RtDevice::QueryAllSensorValue(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 count = obj.value("count").toInt();

    return DeviceProtocol::QueryAllSensorValue(count);
}

QByteArray RtDevice::QueryComboActionVersion()
{
    return DeviceProtocol::QueryComboActionVersion();
}

QByteArray RtDevice::QueryMCUSoftVersion()
{
    return DeviceProtocol::QueryMCUSoftVersion();
}

QByteArray RtDevice::GetPrinterHeadHeight()
{
    return DeviceProtocol::GetPrinterHeadHeight();
}

QByteArray RtDevice::GetPrinterContractSteps()
{
    return DeviceProtocol::GetPrinterContractSteps();
}

QByteArray RtDevice::GetPrinterWaitHeight()
{
    return DeviceProtocol::GetPrinterWaitHeight();
}

QByteArray RtDevice::MoveMotorToResetPos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();

    return DeviceProtocol::MoveMotorToResetPos(mid, speed);
}

QByteArray RtDevice::ResetMotorPos(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();

    return DeviceProtocol::ResetMotorPos(mid, speed);
}

QByteArray RtDevice::MoveForward(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();
    quint32 steps = static_cast<quint32>(obj.value("steps").toDouble());

    return DeviceProtocol::MoveForward(mid, speed, steps);
}

QByteArray RtDevice::MoveBackward(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();
    quint32 steps = static_cast<quint32>(obj.value("steps").toDouble());

    return DeviceProtocol::MoveBackward(mid, speed, steps);
}

QByteArray RtDevice::MoveToCoord(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();
    quint32 coord = static_cast<quint32>(obj.value("coord").toDouble());

    return DeviceProtocol::MoveToCoord(mid, speed, coord);
}

QByteArray RtDevice::MoveToPosid(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 mid = obj.value("mid").toInt();
    quint16 speed = obj.value("speed").toInt();
    quint8 posid = obj.value("posid").toInt();

    return DeviceProtocol::MoveToPosid(mid, speed, posid);
}

QByteArray RtDevice::PumpOnOff(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 pid = obj.value("pid").toInt();
    quint8 on = obj.value("on").toBool() ? 1 : 0;

    return DeviceProtocol::PumpOnOff(pid, on);
}

QByteArray RtDevice::AllPumpOnOff(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 on = obj.value("on").toBool() ? 1 : 0;

    return DeviceProtocol::AllPumpOnOff(on);
}

QByteArray RtDevice::ValveOnOff(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 vid = obj.value("vid").toInt();
    quint8 on = obj.value("on").toBool() ? 1 : 0;

    return DeviceProtocol::ValveOnOff(vid, on);
}

QByteArray RtDevice::AllValveOnOff(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 on = obj.value("on").toBool() ? 1 : 0;

    return DeviceProtocol::AllValveOnOff(on);
}

QByteArray RtDevice::OpenHeater(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::OpenHeater(id);
}

QByteArray RtDevice::CloseHeater(const QJsonValue &value)
{
    QJsonObject obj = value.toObject();
    quint8 id = obj.value("id").toInt();

    return DeviceProtocol::CloseHeater(id);
}

