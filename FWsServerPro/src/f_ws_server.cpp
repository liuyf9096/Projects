#include "../include/f_ws_server.h"
#include "f_ws_server_p.h"
#include "f_network_server.h"

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

//![FWebSocketServer]

FWebSocketServer::FWebSocketServer(QObject *parent)
    : QObject(parent)
    , Dptr(new FWebSocketServerPrivate(this))
{
    qDebug().noquote() << QString("FWebSocketServer created. version:%1").arg("1.1.6");

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
    QMetaObject::invokeMethod(Dptr, "stopServer", Qt::AutoConnection);
}

quint16 FWebSocketServer::listeningPort()
{
    return Dptr->m_listenPort;
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

void FWebSocketServer::setSendingExemptWords(const QStringList &list)
{
    Dptr->m_sendingExemptList = list;
    qDebug() << "[WebSocketServer] set Sending Exempt Words:" << list.join(", ");
}

void FWebSocketServer::setReceivingExemptWords(const QStringList &list)
{
    Dptr->m_receivingExemptList = list;
    qDebug() << "[WebSocketServer] set Receiving Exempt Words:" << list.join(", ");
}

QStringList FWebSocketServer::getConnectedClientId()
{
    return Dptr->m_clientMap.keys();
}

QStringList FWebSocketServer::getConnectedAddress()
{
    QStringList list;
    foreach (auto client_id, Dptr->m_clientMap.keys()) {
        QStringList l = client_id.split(":");
        if (l.count() > 1) {
            list.append(l.first());
        }
    }
    return list;
}

//![FWebSocketServerPrivate]

FWebSocketServerPrivate::FWebSocketServerPrivate(FWebSocketServer *parent) : q_ptr(parent)
{
    m_newPort = 0;
    m_listenPort = 0;

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
    Q_Q(FWebSocketServer);

    if (mServer->isListening()) {
        if (port != m_listenPort || udpKey != m_udpKeyword) {
            stopServer();

            m_newPort = port;
            n_newUdpKey = udpKey;
        } else {
            qDebug() << "Already listening port:" << port;
        }
    } else {
        bool ok = mServer->listen(QHostAddress::AnyIPv4, port);
        if (ok) {
            m_listenPort = port;
            m_udpKeyword = udpKey;
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

                QStringList list = FNetworkServer::GetInstance()->getIPAddress();
                if (list.isEmpty() == false) {
                    QString ip = list.first();
                    QString msg = QString("FWsServer[%1][%2:%3]").arg(udpKey).arg(ip).arg(port);
                    sendUdpMessage(msg);
                }
            }
            emit q->onListenState_signal();
        } else {
            qWarning() << "[WebSocketServer] listen error. port:" << port;
            emit q->onListenStateFail_signal();
        }
    }
}

void FWebSocketServerPrivate::setClientUserName(const QString client_id, const QString userName)
{
    auto client = m_clientMap.value(client_id);
    if (client) {
        client->setProperty("userName", userName);
    }
}

void FWebSocketServerPrivate::stopServer()
{
    qInfo() << "[WebSocketServer] stop server...";

    mServer->close();
    mUdp->close();

    closeAllClinets();
}

void FWebSocketServerPrivate::closeAllClinets()
{
    foreach (QWebSocket *client, m_clientMap) {
        if (client && client->isValid()) {
            client->abort();
        }
    }
    m_clientMap.clear();
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

bool FWebSocketServerPrivate::checkSendingExmptWords(const QString &message)
{
    foreach (const QString &word, m_sendingExemptList) {
        if (message.contains(word)) {
            return false;
        }
    }
    return true;
}

bool FWebSocketServerPrivate::checkReceivingExmptWords(const QString &message)
{
    foreach (const QString &word, m_receivingExemptList) {
        if (message.contains(word)) {
            return false;
        }
    }
    return true;
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
            if (checkSendingExmptWords(message)) {
                qDebug().noquote() << QString("[%1] <<").arg(userName) << message;
            }
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

    if (m_newPort > 0) {
        listen(m_newPort, n_newUdpKey);
        m_newPort = 0;
    }
    m_listenPort = 0;
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

    if (checkReceivingExmptWords(message)) {
        qDebug().noquote() << QString("[%1] >>").arg(userName.isEmpty() ? clientID : userName)
                           << message.simplified();
    }

    emit q->onReceiveMessage_signal(clientID, message);

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            emit q->onReceiveMessageJsonObj_signal(clientID, obj);
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
    foreach (QWebSocket *client, m_clientMap) {
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
