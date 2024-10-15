#include "f_websocket_server.h"
#include "f_websocket_server_p.h"
#include "network/f_network_server.h"

#include <QTimer>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

static const quint16 Rx_Port = 20020;
static const quint16 Tx_Port = 20010;

FWebSocketServer *FWebSocketServer::GetInstance()
{
    static FWebSocketServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new FWebSocketServer();
    }
    return instance;
}

//![FWebSocketServer]

FWebSocketServer::FWebSocketServer(QObject *parent)
    : QObject(parent)
    , Dptr(new FWebSocketServerPrivate(this))
{
    connect(Dptr, &FWebSocketServerPrivate::onSendMsgError_signal,
            this, &FWebSocketServer::onSendMsgError_signal);
}

void FWebSocketServer::listen(quint16 port, const QString &udpKey)
{
    QMetaObject::invokeMethod(Dptr, "listen", Qt::AutoConnection,
                              Q_ARG(quint16, port),
                              Q_ARG(QString, udpKey));
}

void FWebSocketServer::closeServer()
{
    QMetaObject::invokeMethod(Dptr, "closeAllClinets", Qt::AutoConnection);
    QMetaObject::invokeMethod(Dptr, "stopServer", Qt::AutoConnection);
}

void FWebSocketServer::setClientUserName(const QString &client_id, const QString &username)
{
    QMetaObject::invokeMethod(Dptr, "setClientUserName", Qt::AutoConnection,
                              Q_ARG(QString, client_id),
                              Q_ARG(QString, username));
}

void FWebSocketServer::checkAlive(bool on, int timeout_sec)
{
    QMetaObject::invokeMethod(Dptr, "checkAlive", Qt::AutoConnection,
                              Q_ARG(bool, on),
                              Q_ARG(int, timeout_sec));
}

//![Send Message]
void FWebSocketServer::sendMessage(const QString &client_id, const QString &message)
{
    emit Dptr->sendMessage_signal(client_id, message);
}

void FWebSocketServer::sendJsonObject(const QString &client_id, const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    emit Dptr->sendMessage_signal(client_id, data);
}

void FWebSocketServer::sendPacket(const QString &client_id, const JPacket &p)
{
    QJsonObject obj = FJsonRpcParser::encode(p);
    sendJsonObject(client_id, obj);
}

void FWebSocketServer::setUdpPort(quint16 txPort)
{
    Dptr->m_sendPort = txPort;
}

//![FWebSocketServerPrivate]

FWebSocketServerPrivate::FWebSocketServerPrivate(FWebSocketServer *parent) : q_ptr(parent)
{
    mServer = new QWebSocketServer("ReeTooServer", QWebSocketServer::NonSecureMode, this);
    connect(mServer, &QWebSocketServer::newConnection,
            this, &FWebSocketServerPrivate::onNewConnection_slot);
    connect(mServer, &QWebSocketServer::closed,
            this, &FWebSocketServerPrivate::onWebSocketServerClosed_slot);

    connect(this, &FWebSocketServerPrivate::sendMessage_signal,
            this, &FWebSocketServerPrivate::sendMessage_slot, Qt::QueuedConnection);

    m_sendPort = Tx_Port;
    mUdp = new QUdpSocket(this);
    connect(mUdp, &QUdpSocket::readyRead,
            this, &FWebSocketServerPrivate::onReadDatagram_slot);

    m_heartbeatTimeout_sec = 10;
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(1000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &FWebSocketServerPrivate::heartbeatTimer_slot);

    mThread = new QThread();
    connect(mThread, &QThread::finished, mThread, &QThread::deleteLater);
    this->moveToThread(mThread);
    mThread->start();
}

FWebSocketServerPrivate::~FWebSocketServerPrivate()
{
    mServer->close();
    delete mServer;

    mThread->quit();
    mThread->wait();
    delete mThread;
}

void FWebSocketServerPrivate::listen(quint16 port, const QString &udpKey)
{
    bool ok = mServer->listen(QHostAddress::AnyIPv4, port);
    if (ok) {
        qDebug().noquote() << QString("[WebSocketServer] listen to 'AnyIPv4' port: %1 OK.").arg(port);

        if (udpKey.isEmpty() == false) {
            if (mUdp->isValid() == false) {
                bool ok = mUdp->bind(Rx_Port);
                if (ok) {
                    qDebug() << "bind udp success. port:" << Rx_Port;
                } else {
                    qDebug() << "bind udp fail. port:" << Rx_Port;
                }
            }

            QString msg;
            QStringList list = FNetworkServer::GetInstance()->getIPAddress();
            if (list.isEmpty() == false) {
                QString ip = list.first();
                msg = QString("FWsServer[%1][%2:%3]").arg(udpKey).arg(ip).arg(port);
                sendUdpMessage(msg);
            }
        }
    } else {
        qWarning() << "[WebSocketServer] listen error. port:" << port;
    }
}

void FWebSocketServerPrivate::setClientUserName(const QString client_id, const QString userName)
{
    auto client = m_clientMap.value(client_id);
    if (client) {
        client->setProperty("userName", userName);
    }
}

void FWebSocketServerPrivate::closeAllClinets()
{
    for (QWebSocket *client : qAsConst(m_clientMap)) {
        if (client && client->isValid()) {
            delete client;
        }
    }
    m_clientMap.clear();
}

void FWebSocketServerPrivate::stopServer()
{
    mServer->close();
}

void FWebSocketServerPrivate::checkAlive(bool on, int timeout_sec)
{
    if (on) {
        m_heartbeatTimeout_sec = timeout_sec;
        m_heartbeatTimer->start();
        qDebug() << "start heart beat.";
    } else {
        m_heartbeatTimer->stop();
        qDebug() << "stop heart beat.";
    }
}

void FWebSocketServerPrivate::sendUdpMessage(const QString &message)
{
    for (int i = 0; i < 3; ++i) {
        quint16 port = m_sendPort + i;
        qint64 count = mUdp->writeDatagram(message.toUtf8(), QHostAddress::Broadcast, port);
        if (count > 0) {
            qDebug().noquote() << QString("<< [UDP:%1] %2").arg(port).arg(message);
        } else {
            qDebug().noquote() << QString("[UDP:%1] %2 send fail.").arg(port).arg(message);
        }
    }
}

//! [Send]
void FWebSocketServerPrivate::sendMessage_slot(const QString &client_id, const QString &message)
{
    auto client = m_clientMap.value(client_id);
    if (client && client->isValid()) {
        qint64 count = client->sendTextMessage(message);
        if (count > 0) {
            QString userName = client->property("userName").toString();
            if (userName.isEmpty()) {
                userName = client_id;
            }
            qDebug().noquote() << QString("[%1] <<").arg(userName) << message;
        } else {
            qWarning().noquote() << "[WebSocketServer]: send message Failed." << client_id << message;
            emit onSendMsgError_signal(client_id, message);
        }
    } else {
        emit onSendMsgError_signal(client_id, message);
        qWarning().noquote() << "[WebSocketServer]: client is not valid." << client_id << message;
    }
}

//![WebSocket Server]
void FWebSocketServerPrivate::onNewConnection_slot()
{
    QWebSocket *client = mServer->nextPendingConnection();
    if (client == nullptr) {
        return;
    }

    QString address = client->peerAddress().toString();
    quint16 port = client->peerPort();
    QString client_id = QString("%1:%2").arg(address).arg(port);
    client->setProperty("clientID", client_id);
    client->setProperty("heartbeatcount", 0);

    m_clientMap.insert(client_id, client);

    connect(client, &QWebSocket::textMessageReceived,
            this, &FWebSocketServerPrivate::onClientMessageReceived_slot);
    connect(client, &QWebSocket::disconnected,
            this, &FWebSocketServerPrivate::onClientDisconnection_slot);
    connect(client, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
        [=](QAbstractSocket::SocketError error){
        qDebug() << "[WebSocketServer] Client Error." << error;
    });

    qInfo().noquote() << QString("[WebSocketServer] client (%1) connected.").arg(client_id);

    Q_Q(FWebSocketServer);
    emit q->onClientConnected_signal(client_id);
}

void FWebSocketServerPrivate::onWebSocketServerClosed_slot()
{
    qDebug() << "[WebSocketServer] closed.";
}

//![WebSocket Client]
/* receive client message */
void FWebSocketServerPrivate::onClientMessageReceived_slot(const QString &message)
{
    auto client = qobject_cast<QWebSocket *>(sender());
    if (client == nullptr) {
        return;
    }

    Q_Q(FWebSocketServer);

    /* heart beat */
    client->setProperty("heartbeatcount", 0);
    if (message == QStringLiteral("Alive")) {
        client->setProperty("isCheckAlive", true);
        return;
    }

    QString clientID = client->property("clientID").toString();

    /* client name */
    if (message.startsWith("[ClientName]")) {
        qInfo() << ">>" << message;

        QString str = message;
        str.remove("[ClientName]");
        client->setProperty("userName", str);

        emit q->onDetectNewDevice_signal(clientID, str);
        return;
    }

    QString userName = client->property("userName").toString();

    if (clientID.isEmpty()) {
        QString address = client->peerAddress().toString();
        quint16 port = client->peerPort();
        clientID = QString("%1:%2").arg(address).arg(port);
    }

    qDebug().noquote() << QString("[%1] >>").arg(userName.isEmpty() ? clientID : userName)
                       << message.simplified();

    emit q->onReceiveMessage_signal(clientID, message);

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            emit q->onReceiveMassageJsonObj_signal(clientID, obj);
        }
    }
}

/* client disconnect */
void FWebSocketServerPrivate::onClientDisconnection_slot()
{
    auto client = qobject_cast<QWebSocket *>(sender());
    if (client == nullptr) {
        return;
    }

    QString clientID = client->property("clientID").toString();
    QString userName = client->property("userName").toString();

    if (clientID.isEmpty()) {
        QString address = client->peerAddress().toString();
        quint16 port = client->peerPort();
        clientID = QString("%1:%2").arg(address).arg(port);
    }

    client->deleteLater();
    m_clientMap.remove(clientID);

    if (userName.isEmpty()) {
        qInfo().noquote() << QString("! WebSocket client disconnected. %1").arg(clientID);
    } else {
        qInfo().noquote() << QString("! WebSocket client disconnected. %1").arg(userName);
    }

    Q_Q(FWebSocketServer);
    emit q->onClientDisconnected_signal(clientID);
}

void FWebSocketServerPrivate::onReadDatagram_slot()
{
    while (mUdp->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mUdp->receiveDatagram();
        QString message(datagram.data());
        if (!message.isEmpty()) {
            qDebug().noquote() << QString("[UDP:%1] >> %2").arg(Rx_Port).arg(message);
        }
    }
}

void FWebSocketServerPrivate::heartbeatTimer_slot()
{
    foreach (auto client, m_clientMap) {
        bool isCheckAlive = client->property("isCheckAlive").toBool();
        if (isCheckAlive) {
            client->sendTextMessage(QStringLiteral("Alive"));
            bool ok = client->flush();
            if (ok == false) {
                qWarning() << "send Alive false.";
            }

            int heartbeatcount = client->property("heartbeatcount").toInt();
            QString clientID = client->property("clientID").toString();

            heartbeatcount++;
            client->setProperty("heartbeatcount", heartbeatcount);

            if (heartbeatcount > 1) {
                qWarning() << clientID << "heartbeat:" << heartbeatcount;
            }
            if (heartbeatcount >= m_heartbeatTimeout_sec) {
                qWarning() << clientID << "heartbeat timeout, abort client.";
                client->abort();
            }
        }
    }
}
