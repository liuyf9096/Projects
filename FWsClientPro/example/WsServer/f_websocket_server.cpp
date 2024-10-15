#include "f_websocket_server.h"
#include "f_websocket_server_p.h"

#include <QThread>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QDebug>

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
}

void FWebSocketServer::listenPort(quint16 port)
{
    QMetaObject::invokeMethod(Dptr, "listen", Qt::AutoConnection, Q_ARG(quint16, port));
}

void FWebSocketServer::closeServer()
{
    QMetaObject::invokeMethod(Dptr, "closeAllClinets", Qt::AutoConnection);
    QMetaObject::invokeMethod(Dptr, "stopServer", Qt::AutoConnection);
}

//![Send Message]
void FWebSocketServer::sendMessage(const QString &client_id, const QString &message)
{
    emit Dptr->sendMessage_signal(client_id, message);
}

void FWebSocketServer::setDeviceName(const QString &client_id, const QString &deviceName)
{
    QMetaObject::invokeMethod(Dptr, "setClientName", Qt::AutoConnection,
                              Q_ARG(const QString, client_id),
                              Q_ARG(const QString, deviceName));
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

void FWebSocketServerPrivate::listen(quint16 port)
{
    bool ok = mServer->listen(QHostAddress::AnyIPv4, port);
    if (ok) {
        qDebug().noquote() << QString("[WebSocketServer] listen to 'AnyIPv4' port: %1").arg(port);
    } else {
        qWarning() << "[WebSocketServer] listen error. port:" << port;
    }
}

void FWebSocketServerPrivate::setClientName(const QString client_id, const QString userName)
{
    auto client = m_clientMap.value(client_id);
    if (client) {
        client->setProperty("userName", userName);
    }
}

void FWebSocketServerPrivate::closeAllClinets()
{
    foreach (auto client, m_clientMap.values()) {
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

//! [Send]
void FWebSocketServerPrivate::sendMessage_slot(const QString &client_id, const QString &message)
{
    auto client = m_clientMap.value(client_id);
    if (client && client->isValid()) {
        client->sendTextMessage(message);

        QString userName = client->property("userName").toString();
        if (userName.isEmpty()) {
            userName = client_id;
        }
        qDebug().noquote() << QString("<< [%1][send]").arg(userName) << message;

    } else {
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

    m_clientMap.insert(client_id, client);

    connect(client, &QWebSocket::textMessageReceived,
            this, &FWebSocketServerPrivate::onClientMessageReceived_slot);
    connect(client, &QWebSocket::disconnected,
            this, &FWebSocketServerPrivate::onClientDisconnection_slot);

    qInfo().noquote() << "[WebSocketServer] New Connection. id:" << client_id;

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

    QString clientID = client->property("clientID").toString();
    QString userName = client->property("userName").toString();

    if (clientID.isEmpty()) {
        QString address = client->peerAddress().toString();
        quint16 port = client->peerPort();
        clientID = QString("%1:%2").arg(address).arg(port);
    }

    qDebug().noquote() << QString(">>%1[receive]").arg(userName.isEmpty() ? clientID : userName)
                       << message;


    Q_Q(FWebSocketServer);
    emit q->receiveMassage_signal(clientID, message);
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
