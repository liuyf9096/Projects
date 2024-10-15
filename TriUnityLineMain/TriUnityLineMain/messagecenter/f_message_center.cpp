#include "f_message_center.h"

#include "websocket/f_websocket_manager.h"
#include "websocket/f_websocket_server.h"
#include "device/rt_device_manager.h"
#include "module/module_manager.h"
#include "station/station_manager.h"

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

    /* socket To canbus */
    mCanbus = FWebSocketManager::GetInstance()->getSocket("Canbus");
    connect(mCanbus, &FWebSocket::onReceiveMessageObj_signal,
            this, &FMessageCenter::onReceiveCanbusMessage_slot);

    /* socket To ipu */
    mIPU = FWebSocketManager::GetInstance()->getSocket("IPU");
    connect(mIPU, &FWebSocket::onReceiveMessageObj_signal,
            this, &FMessageCenter::onReceiveIPUMessage_slot);

    /* websocket server S1,S2,S3,UI */
    auto wsServer = FWebSocketServer::GetInstance();
    connect(wsServer, &FWebSocketServer::onClientConnected_signal, this, [=](const QString &client_id){
        mStationIpMap.insert(client_id, "unknown");
        emit onClientConnected_signal(client_id, true);
    });
    connect(wsServer, &FWebSocketServer::onDetectNewDevice_signal, this, [=](const QString &client_id, const QString &client_name){
        mStationIpMap.insert(client_id, client_name);
        emit onClientDeviceConnected_signal(client_name, client_id, true);
        if (client_name == "UI") {
            m_ui_client_id = client_id;
            m_isUIConnected = true;
        }
    });
    connect(wsServer, &FWebSocketServer::onClientDisconnected_signal, this, [=](const QString &client_id){
        QString client_name = mStationIpMap.take(client_id);
        if (client_name == "UI") {
            m_ui_client_id.clear();
            m_isUIConnected = false;
        }
        emit onClientConnected_signal(client_id, false);
        emit onClientDeviceConnected_signal(client_name, client_id, false);
    });
    connect(wsServer, &FWebSocketServer::onReceiveMassageJsonObj_signal,
            this, &FMessageCenter::onReceiveClientsMessage_slot);
}

bool FMessageCenter::isIPUConnected()
{
    return mIPU->isConnected();
}

bool FMessageCenter::isCanbusConnected()
{
    return mCanbus->isConnected();
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

quint64 FMessageCenter::sendCanbusMessage(JPacket &request, const QString &username)
{
    Q_UNUSED(username)

    quint64 id = m_canbus_id++;

    request.id = id;
    if (request.device.isEmpty()) {
        request.device = QStringLiteral("CanbusServer");
    }

    m_CanbusPacketMap.insert(id, request);
    mCanbus->sendMessage(request);
    return id;
}

quint64 FMessageCenter::sendUIMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("UI");
    }
    return sendClientMessage(m_ui_client_id, packet);
}

quint64 FMessageCenter::sendIPUMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("LabXpert");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }

    mIPU->sendMessage(packet);
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
        StationManager::GetInstance()->setStationUnited(p.paramsValue.toObject());
        result.resValue = true;
    } else if (p.api == "SetStationInfo") {
        StationManager::GetInstance()->setStationInfo(p.paramsValue.toObject());
        result.resValue = true;
    } else if (p.api == "SetSystemDateTime") {
        handleSystemDateTime(p.paramsValue.toObject());
        result.resValue = true;
    }

    if (!result.resValue.isNull()) {
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
        QProcess::startDetached("hwclock", QStringList() << "-w");
        QProcess::startDetached("sync");
    }
}

void FMessageCenter::onReceiveCanbusMessage_slot(const QJsonObject &obj)
{
    JPacket packet = FJsonRpcParser::decode(obj);
    if (packet.type == PacketType::Notification) {
        if (packet.api == "Notification") {
            auto obj = packet.paramsValue.toObject();
            if (obj.contains("cmd")) {
                int cmd = obj.value("cmd").toInt();
                if (cmd == 0xE042 || cmd == 0xE041) {
                    handleKeyNotification(obj);
                }
            }
        } else {
            emit onReceiveCanbusNotification_signal(packet);
        }
    } else if (m_CanbusPacketMap.contains(packet.id)) {
        auto request = m_CanbusPacketMap.take(packet.id);
        emit onReceiveCanbusPacket_signal(packet, request);
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

void FMessageCenter::onReceiveIPUMessage_slot(const QJsonObject &obj)
{
    qDebug() << "[IPU] >>" << obj;

    JPacket packet = FJsonRpcParser::decode(obj);

    if (packet.type == PacketType::Request) {
        //todo
    } else if (packet.type == PacketType::Result) {
        if (m_WsPacketMap.contains(packet.id)) {
            auto request = m_WsPacketMap.take(packet.id);
            emit onIPUMessageResultPacket_signal(packet, request);
        }
    }
}
