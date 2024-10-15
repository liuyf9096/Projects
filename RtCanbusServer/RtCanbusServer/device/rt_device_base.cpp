#include "rt_device_base.h"
#include "f_common.h"
#include "device_protocol.h"
#include "servers/sql/f_sql_database_manager.h"

#include <QJsonArray>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>

#define SQL_RECORD

/* CanReqPacket */
CanReqPacket::CanReqPacket() : canId(0), cmdId(0) {}

CanReqPacket::CanReqPacket(quint16 can_id, const QString &api, quint16 cmd_u, const QByteArray &data)
    : canId(can_id), api(api), cmdId(cmd_u), data(data) {}

/* WsRequest */
WsRequest::WsRequest() : ws_id(0), can_id(0) {}

WsRequest::WsRequest(const QString &clientid, quint64 wsid, quint16 canid, const JPacket &p)
    : client_id(clientid)
    , ws_id(wsid)
    , can_id(canid)
    , packet(p)
{

}

/* RtDeviceBase */

QStringList RtDeviceBase::ReplyType = { "0", "ACK", "Done", "Fail", "Error", "Passive", "Notification", "Timeout" };

RtDeviceBase::RtDeviceBase(const QString &dev_id, quint16 address, const QString &type, QObject *parent)
    : QObject(parent)
    , m_deviceType(type)
    , m_devID(dev_id)
    , m_address(address)
    , m_can_id(0)
    , mDb(nullptr)
{
#ifdef SQL_RECORD
    mDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (mDb == nullptr) {
        qWarning() << "log database is NOT connected.";
    }
#endif

    m_sqlHistoryTable = QString("%1_history").arg(m_devID);

    init();

    m_sendingTimer = new QTimer(this);
    connect(m_sendingTimer, &QTimer::timeout,
            this, &RtDeviceBase::_sendingCanPacket_slot);

    m_checkSensorTimer = new QTimer(this);
    connect(m_checkSensorTimer, &QTimer::timeout,
            this, &RtDeviceBase::_checkAllSensors_slot);

    m_checkFloaterSensorTimer = new QTimer(this);
    m_checkFloaterSensorTimer->setInterval(1000);
    connect(m_checkFloaterSensorTimer, &QTimer::timeout,
            this, &RtDeviceBase::_checkFloaterSensor_slot);

    /* device <-> websocket */
    g_ws = FMessageCenter::GetInstance();
    connect(g_ws, &FMessageCenter::onConnected_signal, this, [=](const QString &/*client_id*/){
    });
    connect(g_ws, &FMessageCenter::onDisconnected_signal, this, [=](const QString &client_id){
        setCheckSensorEnable(client_id, false);
        setCheckFloaterSensorEnable(client_id, false, 0);
        m_client_sensorCheck_Map.remove(client_id);

        auto list = m_canid_clientid_Map.keys(client_id);
        foreach (auto can_id, list) {
            m_canid_wsReq_Map.remove(can_id);
        }
    });
    connect(g_ws, &FMessageCenter::onReceivePacket_signal,
            this, &RtDeviceBase::handleWSMessage_slot);

    qInfo().noquote() << QString("[%1] address:0x%2 type:%3")
                         .arg(dev_id).arg(address, 2, 16, QChar('0')).arg(type);
}

void RtDeviceBase::init()
{
    m_can_id = 0;
    timeoutCount = 0;

    m_client_sensorCheck_Map.clear();
    m_canid_clientid_Map.clear();
    m_canid_wsReq_Map.clear();
    m_canReqMap.clear();

    m_floaterCount = 0;

    m_sendingList.clear();

#ifdef SQL_RECORD
    if (m_devID != "main") {
        if (mDb != nullptr) {
            mDb->deleteRecord(m_sqlHistoryTable, QJsonObject(), false);
        }
    }
#endif
}

void RtDeviceBase::setUsername(const QString &username)
{
    qInfo().noquote() << QString("[%1] set username: %2.").arg(m_devID, username);
    m_username = username;
}

void RtDeviceBase::setSendMsgInterval(int interval)
{
    qInfo().noquote() << QString("[%1] set send message interval: %2ms.").arg(m_devID).arg(interval);

    if (interval >= 0) {
        m_sendingTimer->start(interval);
    }
}

void RtDeviceBase::setCheckSensorInterval(int interval)
{
    qInfo().noquote() << QString("[%1] set check sensor interval: %2ms.").arg(m_devID).arg(interval);

    if (interval >= 0) {
        m_checkSensorTimer->setInterval(interval);
    }
}

void RtDeviceBase::setCheckSensorEnable(const QString &client_id, bool en)
{
    m_client_sensorCheck_Map.insert(client_id, en);

    setCheckSensorStart();
}

void RtDeviceBase::setCheckSensorStart()
{
    bool check_enable = false;
    foreach (auto en, m_client_sensorCheck_Map) {
        if (en == true) {
            check_enable = true;
            break;
        }
    }
    if (check_enable) {
        m_checkSensorTimer->start();
        qInfo().noquote() << QString("[%1] check sensor start.").arg(m_devID);
    } else {
        m_checkSensorTimer->stop();
        qInfo().noquote() << QString("[%1] check sensor stop.").arg(m_devID);
    }

    m_sensorUpdate = true;
    timeoutCount = 0;
}

void RtDeviceBase::setCheckSensorStop()
{
    m_checkSensorTimer->stop();
    qInfo().noquote() << QString("[%1] check sensor stop.").arg(m_devID);
}

void RtDeviceBase::setSensors(const QJsonArray &arr)
{
    int maxport = 0;

    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject sensorObj = arr.at(i).toObject();
        int port = sensorObj.value("port").toInt();
        QString name = sensorObj.value("name").toString();
        QString description = sensorObj.value("description").toString();
        if (maxport < port) {
            maxport = port;
        }

        if (name.isEmpty() == false) {
            SensorsMap.insert(port, name);
            qDebug().noquote() << QString("port:%1 (%2) %3").arg(port).arg(name, description);
        }
    }
    m_maxport = qMax(arr.count(), maxport);
    qDebug() << "Max port:" << m_maxport;
}

void RtDeviceBase::setFloaterSensors(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject sensorObj = arr.at(i).toObject();
        int port = sensorObj.value("port").toInt();
        QString name = sensorObj.value("name").toString();
        QString description = sensorObj.value("description").toString();

        if (name.isEmpty() == false) {
            FloaterSensorsMap.insert(port, name);
            qDebug().noquote() << QString("floater port:%1 (%2) %3").arg(port).arg(name, description);
        }
    }
}

void RtDeviceBase::setCheckFloaterSensorEnable(const QString &client_id, bool en, int count)
{
    if (en) {
        m_queryClientId = client_id;
        m_checkFloaterSensorTimer->start();
        m_floaterCount = count;
    } else {
        m_checkFloaterSensorTimer->stop();
        m_queryClientId.clear();
    }
}

quint16 RtDeviceBase::cmd_GetAllSensorValue()
{
    QString api = "GetAllSensorValue";
    quint16 cmd_id = DeviceProtocol::CmdId(api);
    QByteArray data = DeviceProtocol::GetAllSensorValue(m_maxport);

    quint16 canid = newId();
    CanReqPacket p(canid, api, cmd_id, data);
    m_sendingList.append(p);
    return canid;
}

quint16 RtDeviceBase::cmd_GetAllFloaterSensorValue(int count)
{
    QString api = "QueryAllFloater";
    quint16 cmd_id = DeviceProtocol::CmdId(api);
    QByteArray data = DeviceProtocol::QueryAllFloater(count);

    quint16 canid = newId();
    CanReqPacket p(canid, api, cmd_id, data);
    m_sendingList.append(p);
    return canid;
}

//! [1] receive upper request | packet -> id, upperid, api, canid ...
void RtDeviceBase::handleWSMessage_slot(const QString &client_id, const QString &dev_id, const JPacket &p)
{
    if (dev_id != "all" && dev_id != m_devID) {
        return;
    }

    QJsonObject paramsObj = p.paramsValue.toObject();
    if (p.api == "SensorCheckEnable")
    {
        bool on = paramsObj.value("enable").toBool();
        setCheckSensorEnable(client_id, on);
        send_ResultOK(client_id, p.id);
    }
    else if (p.api == "CheckSensorStart") {
        setCheckSensorStart();
        send_ResultOK(client_id, p.id);
    }
    else if (p.api == "CheckSensorStop") {
        setCheckSensorStop();
        send_ResultOK(client_id, p.id);
    }
    else if (p.api == "CheckFloaterSensorEnable") {
        bool on = paramsObj.value("enable").toBool();
        int count = paramsObj.value("count").toInt();
        setCheckFloaterSensorEnable(client_id, on, count);
        send_ResultOK(client_id, p.id);
    }
    else
    {
        QString comboFunc;
        quint16 canid = 0;
        if (p.api == "CheckSensorValue") {
            canid = cmd_GetAllSensorValue();
        } else {
            bool ok = handlePacket(canid, p);
            if (ok == false) {
                return;
            }
            if (p.api.startsWith("ComboAction")) {
                if (paramsObj.contains("api")) {
                    comboFunc = paramsObj.value("api").toString();
                }
            }
        }

        m_canid_clientid_Map.insert(canid, client_id);

        WsRequest req(client_id, p.id, canid, p);
        m_canid_wsReq_Map.insert(canid, req);

#ifdef SQL_RECORD
        insertRecordDB(p.id, canid, comboFunc.isEmpty() ? p.api : comboFunc);
#endif
    }
}

void RtDeviceBase::send_ResultOK(const QString &client_id, quint64 id)
{
    JPacket p(PacketType::Result, id);
    p.resValue = true;
    g_ws->sendMessage(client_id, p, m_devID);
}

//! [4] receive canbus message
void RtDeviceBase::handleCanbusMessage_slot(int address, const QJsonObject &obj, const QByteArray &resValue)
{
    if (address != m_address) {
        return;
    }

    int can_id = obj.value("id").toInt();
    int funcid = obj.value("cmd").toInt();
    int state = obj.value("state").toInt();

    /* state */
    if (state >= ReplyType.count()) {
        qWarning() << "can reply state error." << state;
        return;
    }

    //! [6: Notification]
    if (state == CommuType_Notification) {
        handleCanbusNotification(obj, resValue);
        return;
    }

    if (m_canReqMap.contains(can_id) == false){
        qWarning() << "can NOT match can_id." << m_devID << obj << m_canReqMap.count();
        return;
    }

    CanReqPacket canReqPack = m_canReqMap.value(can_id);

#ifdef CAN_VERBOSE
    qDebug().noquote() << QString(">> [CAN receive] canid:%1 (%2)[cmd:0x%3 %4] state:%5")
                          .arg(can_id).arg(m_devID)
                          .arg(funcid, 0, 16).arg(canReqPack.api, ReplyType.at(state));
#endif

    //! [1: ACK]
    if (state == CommuType_ACK) {
        handleReceiveCanAck(can_id, canReqPack.api, obj);
    }
    else
    {
        //! [2: OK 5: Passive]
        if (state == CommuType_Done_OK || state == CommuType_Passive) {
            handleReceiveCanResult(can_id, canReqPack.api, obj, resValue);
        }
        //! [3: Fail]
        else if (state == CommuType_Done_Fail) {
            handleReceiveResultCanFail(can_id, canReqPack.api, obj);
        }
        //! [4: Error]
        else if (state == CommuType_Command_Error) {
            handleReceiveResultCanError(can_id, canReqPack.api, obj);
        }
        //! [Others]
        else {
            qWarning() << "Unknown result type:" << state;
        }
        m_canReqMap.remove(can_id);
    }
}

void RtDeviceBase::handleReceiveCanAck(quint16 canid, const QString &api, const QJsonObject &resObj)
{
    Q_UNUSED(api)
    Q_UNUSED(resObj)

    if (m_canid_wsReq_Map.contains(canid)) {
#ifdef SQL_RECORD
        WsRequest req = m_canid_wsReq_Map.value(canid);
        updateRecordState(req.ws_id, CommuType_ACK);
        updateRecordAckTime(req.ws_id, QTime::currentTime().toString("hh:mm:ss.zzz"));
#endif
    }
}

bool RtDeviceBase::handleReceiveCanResult(quint16 canid, const QString &api, const QJsonObject &resObj, const QByteArray &data)
{
    Q_UNUSED(resObj)

#if 0
    qDebug() << "RtDeviceBase:" << __FUNCTION__ << canid << "request:" << api << resObj << data;
#endif

    if (api == "GetAllSensorValue")
    {
        QByteArray sensor = data.mid(1);
        handleAllSensorData(canid, FCommon::bytesToBits(sensor));
    }
    else if (api == "QueryAllFloater")
    {
        handleAllFloaterSensorData(canid, data);
    }
    else if (m_canid_wsReq_Map.contains(canid))
    {
        WsRequest req = m_canid_wsReq_Map.take(canid);

        JPacket p(PacketType::Result, req.ws_id);
        if (data.isEmpty()) {
            p.resValue = true;
        } else {
            p.resValue = handleResult(req, data);
        }
        g_ws->sendMessage(req.client_id, p, m_devID);

#ifdef SQL_RECORD
        updateRecordState(req.ws_id, CommuType_Done_OK);
        updateRecordStopTime(req.ws_id, QTime::currentTime().toString("hh:mm:ss.zzz"));
#endif
    } else if (m_canReqMap.contains(canid)) {
        handleSelfResult(api, data);
    } else {
        qWarning() << "Can Not Handle Can Result." << canid << m_canid_wsReq_Map.keys();
    }

    if (api == "Reset") {
        init();
        qInfo().noquote() << QString("[%1] Reset Ok.").arg(m_devID);
    }
    return true;
}

void RtDeviceBase::handleReceiveResultCanFail(quint16 canid, const QString &api, const QJsonObject &resObj)
{
    Q_UNUSED(api)
    Q_UNUSED(resObj)

#if 0
    qDebug() << "RtDeviceBase:" << __FUNCTION__ << canid << "request:" << api << resObj;
#endif

    if (m_canid_wsReq_Map.contains(canid))
    {
        WsRequest req = m_canid_wsReq_Map.take(canid);

        JPacket p(PacketType::Error, req.ws_id);
        p.errorCode = 80;
        p.errorMessage = "Command Execute Fail.";
        g_ws->sendMessage(req.client_id, p, m_devID);

#ifdef SQL_RECORD
        updateRecordState(req.ws_id, CommuType_Done_Fail);
#endif
    }
}

void RtDeviceBase::handleReceiveResultCanError(quint16 canid, const QString &api, const QJsonObject &resObj)
{
    Q_UNUSED(api)
    Q_UNUSED(resObj)

#if 0
    qDebug() << "RtDeviceBase:" << __FUNCTION__ << canid << "request:" << api << resObj;
#endif

    if (m_canid_wsReq_Map.contains(canid))
    {
        WsRequest req = m_canid_wsReq_Map.take(canid);

        JPacket p(PacketType::Error, req.ws_id);
        p.errorCode = 104;
        p.errorMessage = "Command Error.";
        g_ws->sendMessage(req.client_id, p, m_devID);

#ifdef SQL_RECORD
        updateRecordState(req.ws_id, CommuType_Command_Error);
#endif
    }
}

void RtDeviceBase::handleCanbusNotification(const QJsonObject &obj, const QByteArray &resValue)
{
#if 0
    qDebug() << "RtDeviceBase:" << __FUNCTION__ << obj << resValue.toHex();
#endif

    QByteArray value = resValue;
    int cmdid = obj.value("cmd").toInt();

    JPacket p(PacketType::Notification);
    p.api = "Notification";

    QJsonObject params(obj);
    params.remove("state");
    params.remove("id");

    if (value.isEmpty() == false) {
        if (cmdid == 0xE042 && value.count() == 5) {
            QDataStream out(&value, QIODevice::ReadOnly);
            out.setByteOrder(QDataStream::LittleEndian);

            quint8 index = 0;
            quint32 duration = 0;
            out >> index >> duration;

            params.insert("index", index);
            params.insert("duration", static_cast<double>(duration));
        } else if (cmdid == 0xE041 && value.count() == 1) {
            params.insert("index", value.toHex().toInt());
        } else if (cmdid == 0xE250 && value.count() == 5) {
            p.api = "GripperStatus";

            QDataStream out(&value, QIODevice::ReadOnly);
            out.setByteOrder(QDataStream::LittleEndian);

            quint8 board_id = 0;
            quint16 function_id = 0;
            quint8 gripper_id = 0;
            quint8 grip_state = 0;

            out >> board_id >> function_id >> gripper_id >> grip_state;
            params.insert("device_id", m_devID);
            params.insert("address", m_address);
            params.insert("board_id", board_id);
            params.insert("function_id", function_id);
            params.insert("gripper_id", gripper_id);
            params.insert("grip_state", grip_state);
        } else if (cmdid == 0xE001 && value.count() >= 6) {
            p.api = "Exception";

            QDataStream out(&value, QIODevice::ReadOnly);
            out.setByteOrder(QDataStream::LittleEndian);

            quint8 board_id = 0;
            quint16 function_id = 0;
            quint8 item_type = 0;
            quint8 item_id = 0;
            quint8 error_type = 0;

            out >> board_id >> function_id >> item_type >> item_id >> error_type;

            params.insert("device_id", m_devID);
            params.insert("address", m_address);
            params.insert("board_id", board_id);
            params.insert("function_id", function_id);
            params.insert("item_type", item_type);
            params.insert("item_id", item_id);
            params.insert("error_type", error_type);
        } else if (cmdid == 0xE002 && value.count() >= 7) {
            p.api = "Exception";

            QDataStream out(&value, QIODevice::ReadOnly);
            out.setByteOrder(QDataStream::LittleEndian);

            quint8 board_id = 0;
            quint16 function_id = 0;
            quint8 index_id = 0;
            quint8 item_type = 0;
            quint8 item_id = 0;
            quint8 error_type = 0;

            out >> board_id >> function_id >> index_id >> item_type >> item_id >> error_type;

            params.insert("device_id", m_devID);
            params.insert("address", m_address);
            params.insert("board_id", board_id);
            params.insert("function_id", function_id);
            params.insert("index_id", index_id);
            params.insert("item_type", item_type);
            params.insert("item_id", item_id);
            params.insert("error_type", error_type);
        }
    }

    p.paramsValue = params;
    g_ws->sendNotification(p, m_devID);
}

void RtDeviceBase::handleAllSensorData(quint16 canid, const QBitArray &bits)
{
    m_sensorUpdate = true;

    QJsonObject obj;
    QMap<int, QString>::const_iterator it = SensorsMap.constBegin();
    while (it != SensorsMap.constEnd()) {
        if (it.key() < bits.count()) {
#if 0
            qDebug().noquote() << QString("port:%1 [%2]").arg(it.key(), 2).arg(it.value())
                               << bits.at(it.key());
#endif
            obj.insert(QString::number(it.key()), bits.at(it.key()));
            m_sensorValueMap.insert(it.value(), bits.at(it.key()));
        }
        ++it;
    }

    JPacket p;
    if (m_canid_wsReq_Map.contains(canid)) {
        WsRequest req = m_canid_wsReq_Map.take(canid);

        p.id = req.ws_id;
        p.type = PacketType::Result;
        p.resValue = obj;
        g_ws->sendMessage(req.client_id, p, m_devID);

#ifdef SQL_RECORD
        updateRecordState(req.ws_id, CommuType_Done_OK);
        updateRecordStopTime(req.ws_id, QTime::currentTime().toString("hh:mm:ss.zzz"));
#endif
    } else {
        p.type = PacketType::Notification;
        p.api = QStringLiteral("SensorValue");
        p.paramsValue = obj;

        auto list = m_client_sensorCheck_Map.keys(true);
        foreach (auto client_id, list) {
            g_ws->sendMessage(client_id, p, m_devID);
        }
    }
}

void RtDeviceBase::handleAllFloaterSensorData(quint16 canid, QByteArray data)
{
    QJsonObject obj;
    if (data.count() >= 2) {
        QDataStream out(&data, QIODevice::ReadOnly);
        out.setByteOrder(QDataStream::LittleEndian);

        quint8 count = 0;
        out >> count;

        QJsonArray arr;
        for (int i = 0; i < count; ++i) {
            quint8 status = 0;
            out >> status;
            arr.append(status);
        }
        QJsonObject resObj;
        for (int i = 0; i < arr.count(); ++i) {
            if (FloaterSensorsMap.contains(i)) {
                resObj.insert(FloaterSensorsMap.value(i), arr.at(i).toInt());
            }
        }

        obj.insert("count", count);
        obj.insert("values", arr);
        obj.insert("floater", resObj);
    }

    JPacket p;
    if (m_canid_wsReq_Map.contains(canid)) {
        WsRequest req = m_canid_wsReq_Map.take(canid);

        p.id = req.ws_id;
        p.type = PacketType::Result;
        p.resValue = obj;
        g_ws->sendMessage(req.client_id, p, m_devID);

#ifdef SQL_RECORD
        updateRecordState(req.ws_id, CommuType_Done_OK);
        updateRecordStopTime(req.ws_id, QTime::currentTime().toString("hh:mm:ss.zzz"));
#endif
    } else {
        p.type = PacketType::Notification;
        p.api = QStringLiteral("FloaterSensorValue");
        p.paramsValue = obj;

        g_ws->sendMessage(m_queryClientId, p, m_devID);
    }
}

/* send command slot */
//! [3] loop: send -> canbus
void RtDeviceBase::_sendingCanPacket_slot()
{
    if (m_canReqMap.count() > 999) {
        qWarning() << "Request Map is FULL.";
        return;
    }

    if (!m_sendingList.isEmpty()) {
        CanReqPacket p = m_sendingList.takeFirst();
        m_canReqMap.insert(p.canId, p);

        emit sendCanMessage_signal(p.canId, m_address, 0, p.cmdId, p.data, m_devID);

        if (p.api != "GetAllSensorValue") {
            qDebug().noquote() << QString("[%1] Send Command:%2(%3), canid:%4, handling count:%5")
                                  .arg(m_devID, p.api).arg(p.subapi).arg(p.canId).arg(m_canReqMap.count());
        }
    }
}

void RtDeviceBase::_checkAllSensors_slot()
{
    if (m_sensorUpdate == false) {
        m_canReqMap.remove(m_lastSensorId);

        timeoutCount++;
        qWarning() << m_devID << "CheckSensor Resend count:" << timeoutCount;
    } else {
        timeoutCount = 0;
    }

    m_sensorUpdate = false;
    m_lastSensorId = cmd_GetAllSensorValue();
}

void RtDeviceBase::_checkFloaterSensor_slot()
{
    cmd_GetAllFloaterSensorValue(m_floaterCount);
}

quint16 RtDeviceBase::newId()
{
    return m_can_id++;
}

/******************************************** DATABASE ********************************************/

void RtDeviceBase::insertRecordDB(quint64 ws_id, quint16 can_id, const QString &api)
{
    if (mDb == nullptr) {
        return;
    }

    QJsonObject setObj;
    setObj.insert("ws_id", static_cast<double>(ws_id));
    setObj.insert("can_id", can_id);
    setObj.insert("api", api);
    setObj.insert("address", m_address);
    setObj.insert("send_time", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz"));

    int func_id = mComboActionMap.value(api);
    if (func_id >= 0) {
        setObj.insert("func_id", mComboActionMap.value(api));
    }

    bool ok = mDb->insertRecord(m_sqlHistoryTable, setObj);
    if (ok == false) {
        qWarning() << "[SQL] insertRecord error.";
    }
}

void RtDeviceBase::updateRecordState(quint64 ws_id, int stateCode)
{
    if (mDb == nullptr) {
        return;
    }

    QJsonObject setObj;
    setObj.insert("state_code", stateCode);
    if (stateCode < ReplyType.count()) {
        setObj.insert("state", ReplyType.at(stateCode));
    }

    QJsonObject whereObj;
    whereObj.insert("ws_id", static_cast<double>(ws_id));

    bool ok = mDb->updateRecord(m_sqlHistoryTable, setObj, whereObj);
    if (ok == false) {
        qWarning() << "[SQL] updateRecord error.";
    }
}

void RtDeviceBase::updateRecordAckTime(quint64 ws_id, const QString &time)
{
    if (mDb == nullptr) {
        return;
    }

    QJsonObject setObj;
    setObj.insert("ack_time", time);

    QJsonObject whereObj;
    whereObj.insert("ws_id", static_cast<double>(ws_id));

    bool ok = mDb->updateRecord(m_sqlHistoryTable, setObj, whereObj);
    if (ok == false) {
        qWarning() << "[SQL] updateRecord ack-time error.";
    }
}

void RtDeviceBase::updateRecordStopTime(quint64 ws_id, const QString &time)
{
    if (mDb == nullptr) {
        return;
    }

    QJsonObject setObj;
    setObj.insert("stop_time", time);

    QJsonObject whereObj;
    whereObj.insert("ws_id", static_cast<double>(ws_id));

    bool ok = mDb->updateRecord(m_sqlHistoryTable, setObj, whereObj);
    if (ok == true) {
        updateRecordDurationTime(ws_id);
    } else {
        qWarning() << "[SQL] updateRecord stop-time error.";
    }
}

void RtDeviceBase::updateRecordDurationTime(quint64 ws_id)
{
    if (mDb == nullptr) {
        return;
    }

    QJsonObject whereObj;
    whereObj.insert("ws_id", static_cast<double>(ws_id));

    QJsonValue value = mDb->selectRecord(m_sqlHistoryTable, whereObj);
    if (value.isArray()) {
        QJsonArray arr = value.toArray();
        if (!arr.isEmpty())
        {
            QJsonObject obj = arr.first().toObject();
            QString ack_time = obj.value("ack_time").toString();
            QString stop_time = obj.value("stop_time").toString();
            if (!ack_time.isEmpty() && !stop_time.isEmpty())
            {
                QTime t1 = QTime::fromString(ack_time, "HH:mm:ss.zzz");
                QTime t2 = QTime::fromString(stop_time, "HH:mm:ss.zzz");
                int sec = t1.secsTo(t2);

                QJsonObject setObj;
                setObj.insert("duration", sec);

                mDb->updateRecord(m_sqlHistoryTable, setObj, whereObj);
            }
        }
    }
}

