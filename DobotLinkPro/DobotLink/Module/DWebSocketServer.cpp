#include "DWebSocketServer.h"

#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonDocument>
#include <QDebug>

DWebSocketServer *DWebSocketServer::getInstance()
{
    static DWebSocketServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new DWebSocketServer();
    }
    return instance;
}

DWebSocketServer::DWebSocketServer(QObject *parent) : QObject(parent)
{
    m_WebSocketServer = new QWebSocketServer("DobotLinkServer", QWebSocketServer::NonSecureMode, this);
    connect(m_WebSocketServer, &QWebSocketServer::newConnection, this, &DWebSocketServer::onNewConnection_slot);
    connect(m_WebSocketServer, &QWebSocketServer::closed, this, &DWebSocketServer::onWebSocketServerClosed_slot);

    bool ok = m_WebSocketServer->listen(QHostAddress::Any, 9090);
    if (ok == false) {
        qDebug() << "[WebSocket-Server] listen error.";
    } else {
        qDebug() << "DobotLink WebSocketServer PORT: [9090], enjoy it." << endl;
    }
}

//![1] Send message
/* 发送消息 */
void DWebSocketServer::sendMessage(quint16 port, QString message)
{
    QWebSocket *pClient = m_clientMap.value(port);

    if (pClient && pClient->isValid()) {
        pClient->sendTextMessage(message);
        qDebug().noquote() << QString("<<%1:[send]").arg(port) << message;
    } else {
        qDebug() << "[WebSocket-Server]: client is not valid. port:" << port << message;
    }
}

void DWebSocketServer::sendMessageObj(QJsonObject obj)
{
    quint16 port = 0;
    if (obj.contains("WSport")) {
        port = static_cast<quint16>(obj.value("WSport").toInt(0));
        obj.remove("WSport");
    }

    QJsonDocument document;
    document.setObject(obj);

    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    this->sendMessage(port, QString(byteArray));
}

void DWebSocketServer::closeServer()
{
    m_WebSocketServer->close();
    qDeleteAll(m_clientMap.values());
}

//![2] WebSocketServer
void DWebSocketServer::onNewConnection_slot()
{
    QWebSocket *pClient = m_WebSocketServer->nextPendingConnection();

    if (pClient) {
        qDebug() << "DWebSocket: New Connection. address:"
                 << pClient->peerAddress().toString()
                 << "name:" << pClient->peerName()
                 << "port:" << pClient->peerPort();

        connect(pClient, &QWebSocket::textMessageReceived,
                this, &DWebSocketServer::onClientMessageReceived_slot);
        connect(pClient, &QWebSocket::disconnected, this,
                &DWebSocketServer::onClientDisconnection_slot);

        m_clientMap.insert(pClient->peerPort(), pClient);
    }
}

void DWebSocketServer::onWebSocketServerClosed_slot()
{
    qDebug() << "[WebSocket-Server] closed.";
}

//![3] WebSocketClient
/* 收到信息*/
void DWebSocketServer::onClientMessageReceived_slot(QString message)
{
    auto pClient = qobject_cast<QWebSocket *>(sender());

    qDebug().noquote() << QString(">>%1:[receive]").arg(pClient->peerPort()) << message;
    emit receiveMassage_signal(pClient->peerPort(), message);
}

/* 客户端断开连接 */
void DWebSocketServer::onClientDisconnection_slot()
{
    auto pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clientMap.remove(pClient->peerPort());
        qDebug() << "! DWebSocket client disconnected. port:"
                 << pClient->peerPort();
        emit clientClose_signal(pClient->peerPort());
    }
}

