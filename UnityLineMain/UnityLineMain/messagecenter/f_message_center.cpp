#include "f_message_center.h"

#include "websocket/f_websocket_manager.h"
#include "websocket/f_websocket_server.h"
#include "device/rt_device_manager.h"
#include "module/module_manager.h"
#include "station/station_manager.h"
#include "exception/exception_center.h"
#include "cart/carts_manager.h"
#include "f_common.h"

#include <QMetaType>
#include <QDebug>
#include <QProcess>

FMessageCenter *FMessageCenter::GetInstance()
{
    static FMessageCenter *instance = nullptr;
    if (instance == nullptr) {
        instance = new FMessageCenter();
    }
    return instance;
}

FMessageCenter::FMessageCenter(QObject *parent)
    : QObject(parent)
    , m_canbus_id(0)
    , m_ws_id(0)
{
    qRegisterMetaType<JPacket>("JPacket");

    QJsonObject vObj;
    vObj.insert("version", FCommon::appVersion());
    vObj.insert("releaseDate", FCommon::releaseDate());
    vObj.insert("major", FCommon::Version_major);
    vObj.insert("minor", FCommon::Version_minor);
    vObj.insert("patch", FCommon::Version_patch);
    m_versionObj.insert("United", vObj);

    /* socket To canbus */
    mCanbus = FWebSocketManager::GetInstance()->getSocket("Canbus");
    connect(mCanbus, &FWebSocket::onConnected_signal,
            this, &FMessageCenter::onCanbusConnected_slot);
    connect(mCanbus, &FWebSocket::onReceiveMessageObj_signal,
            this, &FMessageCenter::onReceiveCanbusMessage_slot);

    /* socket To DMU */
    mDMU = FWebSocketManager::GetInstance()->getSocket("DMU");
    connect(mDMU, &FWebSocket::onConnected_signal,
            this, &FMessageCenter::onDMUConnected_slot);
    connect(mDMU, &FWebSocket::onReceiveMessageObj_signal,
            this, &FMessageCenter::onReceiveDMUMessage_slot);
    connect(mDMU, &FWebSocket::onDisconnected_signal,
            this, &FMessageCenter::onDMUDisonnected_slot);
    connect(mDMU, &FWebSocket::onNewServerAddress_signal,
            this, &FMessageCenter::onDMUNewServerAddress_slot);

    /* websocket server S1,S2,S3,UI */
    auto wsServer = FWebSocketServer::GetInstance();
    connect(wsServer, &FWebSocketServer::onClientConnected_signal, this, [=](const QString &client_id){
        mStationIpMap.insert(client_id, "unknown");
        emit onClientConnected_signal(client_id, true);
    });
    connect(wsServer, &FWebSocketServer::onDetectNewDevice_signal, this, [=](const QString &client_id, const QString &client_name){
        mStationIpMap.insert(client_id, client_name);
        emit onClientDeviceConnected_signal(client_name, client_id, true);
        if (client_name == "UnitedUI") {
            m_ui_client_id = client_id;
            m_isUIConnected = true;
        }
    });
    connect(wsServer, &FWebSocketServer::onClientDisconnected_signal, this, [=](const QString &client_id){
        QString client_name = mStationIpMap.take(client_id);
        if (client_name == "UnitedUI") {
            m_ui_client_id.clear();
            m_isUIConnected = false;
        }
        emit onClientConnected_signal(client_id, false);
        emit onClientDeviceConnected_signal(client_name, client_id, false);
    });
    connect(wsServer, &FWebSocketServer::onReceiveMessageJsonObj_signal,
            this, &FMessageCenter::onReceiveClientsMessage_slot);
}

bool FMessageCenter::isDMUConnected()
{
    return mDMU->isConnected();
}

bool FMessageCenter::isCanbusConnected()
{
    return mCanbus->isConnected();
}

bool FMessageCenter::isUnitedUIConnected()
{
    return m_isUIConnected;
}

quint64 FMessageCenter::sendClientMessage(const QString &client_id, JPacket &packet)
{
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }
    if (!client_id.isEmpty()) {
        FWebSocketServer::GetInstance()->sendPacket(client_id, packet);
    }
    return packet.id;
}

quint64 FMessageCenter::sendCanbusMessage(JPacket &packet, const QString &username)
{
    Q_UNUSED(username)

    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("CanbusServer");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_canbus_id++;
        m_CanbusPacketMap.insert(packet.id, packet);
    }

    mCanbus->sendMessage(packet);
    return packet.id;
}

quint64 FMessageCenter::sendUIMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("UnitedUI");
    }
    return sendClientMessage(m_ui_client_id, packet);
}

quint64 FMessageCenter::sendDMUMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("LabXpert");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }

    mDMU->sendMessage(packet);
    return packet.id;
}

void FMessageCenter::onReceiveClientsMessage_slot(const QString &client_id, const QJsonObject &obj)
{
    JPacket packet = FJsonRpcParser::decode(obj);
    if (client_id == m_ui_client_id) {
        handleUIpacket(packet);
        return;
    }

    if (packet.type == PacketType::Notification) {
        emit onClientsMessageNotification_signal(client_id, packet);
    } else if (packet.type == PacketType::Request) {
        emit onClientsMessageRequestPacket_signal(client_id, packet);
    } else if (packet.type == PacketType::Result) {
        auto request = m_WsPacketMap.take(packet.id);
        emit onClientsMessageResultPacket_signal(client_id, packet, request);
    } else if (packet.type == PacketType::Error) {
        auto request = m_WsPacketMap.take(packet.id);
        emit onClientsMessageErrorPacket_signal(client_id, packet, request);
    }
}

void FMessageCenter::handleUIpacket(const JPacket &p)
{
    JPacket result(PacketType::Result, p.id);

    if (p.api == "QueryStationSetup") {
        QJsonObject obj;
        QJsonArray arr = StationManager::GetInstance()->getStationStatus();
        obj.insert("setup", arr);
        result.resValue = obj;
    } else if (p.api == "QueryStationStatus") {
        QJsonObject obj;
        QJsonArray arr = StationManager::GetInstance()->getStationStatus();
        obj.insert("status", arr);
        result.resValue = obj;
    } else if (p.api == "SetStationUnited") {
        if (CartsManager::GetInstance()->isCartsAllBusy()) {
            result.type = PacketType::Error;
        } else {
            StationManager::GetInstance()->setStationUnited(p.paramsValue.toObject());
            result.resValue = true;
        }
    } else if (p.api == "SetStationInfo") {
        StationManager::GetInstance()->setStationInfo(p.paramsValue.toObject());
        result.resValue = true;
    } else if (p.api == "SetSystemDateTime") {
        handleSystemDateTime(p.paramsValue.toObject());
        result.resValue = true;
    } else if (p.module == "Version") {
        if (p.api == "GetVersion") {
            result.resValue = m_versionObj;
        }
    } else if (p.api == "GetException") {
        QJsonArray arr = ExceptionCenter::GetInstance()->getAllException();
        QJsonObject obj;
        obj.insert("exception", arr);
        obj.insert("dev_id", "United");
        result.resValue = obj;
    }

    if (!result.resValue.isNull() || result.type == PacketType::Error) {
        sendUIMessage(result);
    }
}

void FMessageCenter::handleSystemDateTime(const QJsonObject &obj)
{
    QString datetime = obj.value("datetime").toString();
    if (!datetime.isEmpty()) {
        if (datetime.contains(".")) {
            datetime = datetime.left(datetime.indexOf("."));
        }

        QProcess::startDetached("date", QStringList() << "-s" << datetime);

        if (FCommon::GetInstance()->getConfigValue("general", "timesync").toBool() == true) {
            QProcess::startDetached("hwclock", QStringList() << "-systohc");
    //        QProcess::startDetached("hwclock", QStringList() << "-w");
    //        QProcess::startDetached("sync");
        }
    }
}

void FMessageCenter::handleCanbusVersion(const JPacket &result, const JPacket &request)
{
    QJsonObject obj = request.resValue.toObject();
    QJsonObject vObj;
    vObj.insert("version", obj.value("version").toString());
    vObj.insert("major", obj.value("major").toInt());
    vObj.insert("minor", obj.value("minor").toInt());
    vObj.insert("patch", obj.value("patch").toInt());

    if (result.api == "QueryComboActionVersion") {
        m_versionObj.insert("ComboAction", vObj);
    } else if (result.api == "QueryMCUSoftVersion") {
        m_versionObj.insert("MCU", vObj);
    }
}

void FMessageCenter::onReceiveCanbusMessage_slot(const QJsonObject &obj)
{
    JPacket packet = FJsonRpcParser::decode(obj);
    if (packet.type == PacketType::Notification) {
        if (packet.api == "Notification") {
            handleKeyNotification(obj);
        } else if (packet.api == "Exception") {
            handleCanbusException(packet);
        } else {
            emit onReceiveCanbusNotification_signal(packet);
        }
    } else if (m_CanbusPacketMap.contains(packet.id)) {
        auto request = m_CanbusPacketMap.take(packet.id);
        if (request.api == "QueryComboActionVersion"
                || request.api == "QueryMCUSoftVersion") {
            handleCanbusVersion(request, packet);
        } else {
            emit onReceiveCanbusPacket_signal(packet, request);
        }
    } else {
        qDebug() << "can NOT handle obj." << obj;
    }
}

void FMessageCenter::handleKeyNotification(const QJsonObject &obj)
{
    int cmd = obj.value("cmd").toInt();
    if (cmd == 0xE042) {
        int index = obj.value("index").toInt();
        quint32 duration = static_cast<quint32>(obj.value("duration").toDouble());

        if (index == 4) {
            if (duration < 800) {
                qInfo() << "Key: Start Short";
            } else {
                qInfo() << "Key: Start Long";
            }
        } else if (index == 5) {
            if (duration < 800) {
                qInfo() << "Key: Silent Short";
            } else {
                qInfo() << "Key: Silent Long";
            }
        } else if (index == 6) {
            if (duration < 800) {
                qInfo() << "Key: Reset Short";
            } else {
                qInfo() << "Key: Reset Long";

                RtDeviceManager::GetInstance()->sendAllDeviceReset();
                DModuleManager::GetInstance()->reset();  
            }
        }
    } else if (cmd == 0xE041) {
        int index = obj.value("index").toInt();
        if (index == 4) {
            qInfo() << "Key: Start";
        } else if (index == 5) {
            qInfo() << "Key: Silent";
        } else if (index == 6) {
            qInfo() << "Key: Reset";
        }
    }
}

void FMessageCenter::handleCanbusException(JPacket &packet)
{
    QJsonObject obj = packet.paramsValue.toObject();

#if 0
    int board_id = obj.value("board_id").toInt();
    int cmd = obj.value("cmd").toInt();
    int item_id = obj.value("item_id").toInt();
#endif

    int function_id = obj.value("function_id").toInt();
    int index_id = obj.value("index_id").toInt();
    int item_type_id = obj.value("item_type").toInt();
    QString item_type = ExceptionCenter::getItemTypeS(item_type_id - 1);

    int error_type_id = obj.value("error_type").toInt();
    QString error_type = ExceptionCenter::getErrorTypeS(item_type_id, error_type_id);

    obj.insert("item_type", item_type);
    obj.insert("item_type_id", item_type_id);
    obj.insert("error_type", error_type);
    obj.insert("error_type_id", error_type_id);
    obj.insert("source", "device");

    int board_num = RtDeviceManager::GetInstance()->getBoardNum(packet.module);

    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    QJsonArray arr = db->selectRecord("canbus_exception", QJsonObject({{"function_id", function_id}}));
    if (arr.count() > 0) {
        QJsonObject eObj = arr.first().toObject();
        QString error_id = eObj.value("error_code").toString();
        QString message = eObj.value("title").toString();
        QString solution = eObj.value("description").toString();
        obj.insert("error_id", error_id);
        obj.insert("message", message);
        obj.insert("solution", solution);
        obj.insert("level", 1);
    } else {
        QString error_id = QString("%1%2").arg(board_num, 2, 10, QChar('0')).arg(function_id, 3, 10, QChar('0'));
        QString error_code = QString("%1%2").arg(error_id).arg(index_id, 2, 10, QChar('0'));
        obj.insert("error_id", error_id);
        obj.insert("error_code", error_code);
        obj.insert("level", 2);
    }
    obj.insert("dev_id", "United");
    QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    obj.insert("datetime", datetime);

    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;
    sendUIMessage(p);
}

void FMessageCenter::onReceiveDMUMessage_slot(const QJsonObject &obj)
{
//    qDebug() << "[DMU] >>" << obj;

    JPacket packet = FJsonRpcParser::decode(obj);

    if (packet.type == PacketType::Request) {
        if (packet.api == "Retest") {
            RtSampleManager::GetInstance()->setSampleReviewTask(packet);
        } else if (packet.api == "GetDeviceInfo") {
            JPacket p(PacketType::Result, packet.id);
            QJsonObject obj;
            obj.insert("model", "United");
            obj.insert("id", "United");
            p.resValue = obj;
            FMessageCenter::GetInstance()->sendDMUMessage(p);
        }
    } else if (packet.type == PacketType::Result) {
        if (m_WsPacketMap.contains(packet.id)) {
            auto request = m_WsPacketMap.take(packet.id);
            emit onDMUMessageResultPacket_signal(packet, request);
        }
    }
}

void FMessageCenter::onCanbusConnected_slot()
{
    JPacket p(PacketType::Request);
    p.module = "track";
    p.api = "QueryComboActionVersion";
    sendCanbusMessage(p);
    p.api = "QueryMCUSoftVersion";
    sendCanbusMessage(p);
}

void FMessageCenter::onDMUConnected_slot()
{
    ExceptionCenter::GetInstance()->removeException("DMU_Lost_Connection");
}

void FMessageCenter::onDMUDisonnected_slot()
{
    ExceptionCenter::GetInstance()->addException("main", "DMU_Lost_Connection",
                                                 Exception_Type::UserCode);
}

void FMessageCenter::onDMUNewServerAddress_slot(const QString &address, quint16 port)
{
    if (mDMU->address() != address || mDMU->port() != port) {
        mDMU->setServerAddressPort(address, port);

        FCommon::setConfigFileValue("DMU", "websocket", "address", address);
        FCommon::setConfigFileValue("DMU", "websocket", "port", port);
    }
}
