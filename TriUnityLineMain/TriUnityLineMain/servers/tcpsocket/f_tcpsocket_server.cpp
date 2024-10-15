#include "f_tcpsocket_server.h"
#include "f_tcpsocket_server_p.h"

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QDebug>

#define PORT (9090)

FTcpSocketServer *FTcpSocketServer::GetInstance()
{
    static FTcpSocketServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new FTcpSocketServer();
    }
    return instance;
}

//![RtTcpSocketServer]

FTcpSocketServer::FTcpSocketServer(QObject *parent)
    : QObject(parent)
    , Dptr(new FTcpSocketServerPrivate(this))
{
    qDebug() << "RtTcpSocketServer creating...";
    QMetaObject::invokeMethod(Dptr, "listen", Qt::AutoConnection, Q_ARG(quint16, PORT));
}

//![Send Message]
void FTcpSocketServer::sendMessage(const QString &dev_id, const QString &message)
{
    emit Dptr->sendMessage_signal(dev_id, message);
}

void FTcpSocketServer::setDeviceName(const QString &dev_id, const QString &deviceName)
{
    QMetaObject::invokeMethod(Dptr, "setClientName", Qt::AutoConnection,
                              Q_ARG(const QString, dev_id),
                              Q_ARG(const QString, deviceName));
}

void FTcpSocketServer::closeServer()
{
    QMetaObject::invokeMethod(Dptr, "closeAllClinets", Qt::AutoConnection);
    QMetaObject::invokeMethod(Dptr, "stopServer", Qt::AutoConnection);
}

//![RtTcpSocketServerPrivate]

FTcpSocketServerPrivate::FTcpSocketServerPrivate(FTcpSocketServer *parent) : q_ptr(parent)
{
    mServer = new QTcpServer(this);
    connect(mServer, &QTcpServer::newConnection,
            this, &FTcpSocketServerPrivate::onNewConnection_slot);

    connect(this, &FTcpSocketServerPrivate::sendMessage_signal,
            this, &FTcpSocketServerPrivate::sendMessage_slot, Qt::QueuedConnection);

    mThread = new QThread();
    connect(mThread, &QThread::finished, mThread, &QThread::deleteLater);
    this->moveToThread(mThread);
    mThread->start();
}

FTcpSocketServerPrivate::~FTcpSocketServerPrivate()
{
    mServer->close();
    delete mServer;

    mThread->quit();
    mThread->wait();
    delete mThread;
}

void FTcpSocketServerPrivate::listen(quint16 port)
{
    bool ok = mServer->listen(QHostAddress::AnyIPv4, port);
    if (ok) {
        qDebug().noquote() << QString("[TCPSocketServer] listen to 'AnyIPv4' port: %1").arg(PORT);
    } else {
        qWarning() << "[TCPSocketServer] listen error.";
    }
}

void FTcpSocketServerPrivate::setClientName(const QString dev_id, const QString userName)
{
    auto client = m_clientMap.value(dev_id);
    if (client) {
        client->setProperty("userName", userName);
    }
}

void FTcpSocketServerPrivate::closeAllClinets()
{
    foreach (auto client, m_clientMap.values()) {
        if (client && client->isValid()) {
            delete client;
        }
    }
    m_clientMap.clear();
}

void FTcpSocketServerPrivate::stopServer()
{
    mServer->close();
}

void FTcpSocketServerPrivate::sendMessage_slot(const QString &dev_id, const QString &message)
{
    auto client = m_clientMap.value(dev_id);
    if (client && client->isValid()) {
        QByteArray sendArray;
        sendArray.append(message);
        client->write(sendArray);

        QString userName = client->property("userName").toString();
        if (userName.isEmpty()) {
            qDebug().noquote() << QString("<<%1[send]").arg(dev_id) << message;
        } else {
            qDebug().noquote() << QString("<<%1[send]").arg(userName) << message;
        }
    } else {
        qWarning() << "[TcpSocketServer]: client is not valid." << dev_id << message;
    }
}

void FTcpSocketServerPrivate::onNewConnection_slot()
{
    QTcpSocket *client = mServer->nextPendingConnection();
    if (client == nullptr) {
        return;
    }

    QString address = client->peerAddress().toString();
    quint16 port = client->peerPort();
    QString client_id = QString("%1:%2").arg(address).arg(port);
    client->setProperty("clientID", client_id);
    m_clientMap.insert(client_id, client);

    connect(client, &QTcpSocket::readyRead,
            this, &FTcpSocketServerPrivate::onClientMessageReceived_slot);
    connect(client, &QTcpSocket::disconnected,
            this, &FTcpSocketServerPrivate::onClientDisconnection_slot);

    qInfo().noquote() << "[TcpSocketServer] New Connection. id:" << client_id;

    Q_Q(FTcpSocketServer);
    emit q->onClientConnected_signal(client_id);
}

void FTcpSocketServerPrivate::onClientMessageReceived_slot()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    if (client == nullptr) {
        return;
    }

    QByteArray message = client->readAll();
    QString clientID = client->property("clientID").toString();
    QString userName = client->property("userName").toString();

    if (clientID.isEmpty()) {
        QString address = client->peerAddress().toString();
        quint16 port = client->peerPort();
        clientID = QString("%1:%2").arg(address).arg(port);
    }

    if (userName.isEmpty()) {
        qDebug().noquote() << QString(">>%1[receive]").arg(clientID) << message.simplified();
    } else {
        qDebug().noquote() << QString(">>%1[receive]").arg(userName) << message.simplified();
    }

    Q_Q(FTcpSocketServer);
    emit q->receiveMassage_signal(clientID, message);
}

void FTcpSocketServerPrivate::onClientDisconnection_slot()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
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

    qInfo() << "! TCPSocket client disconnected. id:" << clientID;

    Q_Q(FTcpSocketServer);
    emit q->onClientDisconnected_signal(clientID);
}
