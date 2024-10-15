#include "f_message_center.h"

#include "servers/websocket/f_websocket_server.h"
#include "f_common.h"

#include <QJsonDocument>
#include <QDebug>

FMessageCenter *FMessageCenter::GetInstance()
{
    static FMessageCenter *instance = nullptr;
    if (instance == nullptr) {
        instance = new FMessageCenter();
    }
    return instance;
}

FMessageCenter::FMessageCenter(QObject *parent) : QObject(parent)
{
    auto server = FWebSocketServer::GetInstance();
    server->setSendingExemptWords(QStringList() << "SensorValue");

    connect(server, &FWebSocketServer::onClientConnected_signal, this, [=](const QString &client_id){
        mClientMap.insert(client_id, client_id);
        emit onConnected_signal(client_id);
    });
    connect(server, &FWebSocketServer::onClientDisconnected_signal, this, [=](const QString &client_id){
        mClientMap.remove(client_id);
        emit onDisconnected_signal(client_id);
    });
    connect(server, &FWebSocketServer::onReceiveMessageJsonObj_signal,
            this, &FMessageCenter::handleWSmessage_slot);
    connect(server, &FWebSocketServer::onReceiveMessage_signal, this,
            [=](const QString /*&client_id*/, const QString /*&message*/){
//        qDebug().noquote() << QString("?? >> [%1]").arg(client_id) << message;
    });
}

void FMessageCenter::handleWSmessage_slot(const QString &client_id, const QJsonObject &msgObj)
{
    JPacket packet = FJsonRpcParser::decode(msgObj);
    if (packet.device == "CanbusServer") {
        if (packet.module == "Version") {
            if (packet.api == "GetVersion") {
                JPacket p(PacketType::Result, packet.id);
                QJsonObject obj;
                obj.insert("version", FCommon::appVersion());
                obj.insert("releasedate", FCommon::releaseDate());
                p.resValue = obj;
                sendMessage(client_id, p, client_id);
            }
        } else {
            emit onReceiveJsonObj_signal(client_id, packet.module, msgObj);
            emit onReceivePacket_signal(client_id, packet.module, packet);
        }
    } else {
        qWarning().noquote() << QString("?? >> [%1]").arg(client_id) << msgObj;
    }
}

void FMessageCenter::sendMessage(const QString &clientID, JPacket &p, const QString &dev_id)
{
    p.device = QStringLiteral("CanbusServer");
    p.module = dev_id;

    QJsonObject obj = FJsonRpcParser::encode(p);
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

#ifdef CENTER_VERBOSE
    qDebug().noquote() << QString("<< [%1]").arg(dev_id) << data;
#endif

    FWebSocketServer::GetInstance()->sendMessage(clientID, data);
}

void FMessageCenter::sendNotification(JPacket &p, const QString &dev_id)
{
    for (auto & client_id : mClientMap) {
        sendMessage(client_id, p, dev_id);
    }
}
