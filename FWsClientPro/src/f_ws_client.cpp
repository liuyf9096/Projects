#include "include/f_ws_client.h"
#include "f_ws_client_p.h"
#include "include/f_jsonrpc_parser.h"

#include <QWebSocket>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QNetworkDatagram>
#include <QSslError>
#include <QNetworkProxy>
#include <QUrl>
#include <QTimer>
#include <QDebug>

//![FWsClient]
static const int DEFAULT_RECONNECT_MS = 1000;

FWsClient::FWsClient(QObject *parent)
    : QObject(parent)
    , Dptr(new FWsClientPrivate(this))
{
    qDebug().noquote() << QString("FWsClient created. version:%1").arg("2.1.11");
}

bool FWsClient::isConnected()
{
    Q_D(FWsClient);
    return d->m_isConnected;
}

QString FWsClient::address()
{
    Q_D(FWsClient);
    return d->m_address;
}

quint16 FWsClient::port()
{
    Q_D(FWsClient);
    return d->m_port;
}

void FWsClient::setServerAddress(const QString &address)
{
    Q_D(FWsClient);
    d->m_address = address;
    d->m_url = QString("ws://%1:%2").arg(address).arg(d->m_port);
}

void FWsClient::setServerWSPort(quint16 port)
{
    Q_D(FWsClient);
    d->m_port = port;
    d->m_url = QString("ws://%1:%2").arg(d->m_address).arg(port);
}

void FWsClient::setServerAddressPort(const QString &address, quint16 port)
{
    Q_D(FWsClient);
    d->m_address = address;
    d->m_port = port;
    d->m_url = QString("ws://%1:%2").arg(address).arg(port);
}

void FWsClient::setClientName(const QString &name)
{
    Q_D(FWsClient);
    d->m_clientName = name;
}

void FWsClient::autoDetectServer(const QString &keyword, quint16 udp_port)
{
    Q_D(FWsClient);
    d->m_keyword = keyword;
    d->startUdp(udp_port);
}

void FWsClient::connectServer(bool isAutoReconnect, const QString &serverName)
{
    Q_D(FWsClient);
    connectServer(d->m_address, d->m_port, isAutoReconnect, serverName);
}

void FWsClient::connectServer(const QString &address, quint16 port, bool isAutoReconnect, const QString &serverName)
{
    Q_D(FWsClient);

    d->m_serverName = serverName;

    if (d->m_isConnected == true) {
        if (d->m_address == address && d->m_port == port) {
            qInfo() << "WsClient is already connected.";
            return;
        } else {
            disconnectServer();
            qWarning() << "WsClient: new address and port, disconnect the existing connection first.";
        }
    } else {
        d->m_address = address;
        d->m_port = port;
        d->m_url = QString("ws://%1:%2").arg(address).arg(port);
        d->m_isAutoReconnect = isAutoReconnect;

        if (d->m_verbose) {
            qInfo() << "WsClient is connecting to server:" << serverName << d->m_url;
        }

        /* connect url */
        d->mSocket->open(QUrl(d->m_url));
    }
}

void FWsClient::disconnectServer()
{
    Q_D(FWsClient);
    d->m_isConnecting = false;
    d->m_isConnected = false;
    d->m_isAutoReconnect = false;
    d->mSocket->abort();
    d->m_reconnectTimer->stop();
}

void FWsClient::setAutoReconnectInterval(int interval)
{
    if (interval < 20) {
        qWarning() << "WsClient auto reconnect timer interval is too small." << QString("%1ms").arg(interval);
    } else {
        Q_D(FWsClient);
        d->m_reconnectTimer->setInterval(interval);
        qInfo() << "WsClient: set client auto reconnect timer interval:" << QString("%1ms").arg(interval);
    }
}

void FWsClient::checkAlive(bool on, int timeout_sec)
{
    Q_D(FWsClient);
    d->checkAlive(on, timeout_sec);
}

void FWsClient::sendMessage(const QJsonObject &obj)
{
    QByteArray data = FJsonRpcParser::convertObjToByte(obj);

    Q_D(FWsClient);
    emit d->sendMessage_signal(QString(data));
}

void FWsClient::sendMessage(const QString &message)
{
    Q_D(FWsClient);
    emit d->sendMessage_signal(message);
}

void FWsClient::sendMessage(const JPacket &packet)
{
    QJsonObject obj = FJsonRpcParser::encode(packet);
    sendMessage(obj);
}

void FWsClient::setVerboseOn(bool on)
{
    Q_D(FWsClient);
    d->m_verbose = on;
}

//![FWsClientPrivate]

FWsClientPrivate::FWsClientPrivate(FWsClient *parent)
    : q_ptr(parent)
    , m_verbose(true)
    , m_isConnecting(false)
    , m_isConnected(false)
    , m_isAutoReconnect(false)
    , m_heartbeatCount(0)
{
    m_heartbeatTimeout_sec = 10;

    QNetworkProxyFactory::setUseSystemConfiguration(false);

    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    mSocket = new QWebSocket();
    mSocket->setParent(this);
    mSocket->setProxy(QNetworkProxy::NoProxy);
    connect(mSocket, &QWebSocket::connected, this, &FWsClientPrivate::onConnected_slot);
    connect(mSocket, &QWebSocket::disconnected, this, &FWsClientPrivate::onDisconnected_slot);
    connect(mSocket, &QWebSocket::aboutToClose, [=](){
        qWarning() << "WsClient aboutToClose:" << mSocket->errorString() << mSocket->closeCode() << mSocket->closeReason();
    });
    connect(mSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &FWsClientPrivate::onSslErrors_slot);
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            [=](QAbstractSocket::SocketError error){
        qWarning() << "WsClient Error:" << error;
    });
    connect(mSocket, &QWebSocket::textMessageReceived, this, &FWsClientPrivate::onMessageReceived_slot);

    mUdp = new QUdpSocket(this);
    connect(mUdp, &QUdpSocket::readyRead,
            this, &FWsClientPrivate::onReadDatagram_slot);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(DEFAULT_RECONNECT_MS);
    connect(m_reconnectTimer, &QTimer::timeout, this, &FWsClientPrivate::onReConnectedTimer_slot);

    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(1000);
    connect(m_keepAliveTimer, &QTimer::timeout, this, &FWsClientPrivate::onKeepAliveTimer_slot);

    connect(this, &FWsClientPrivate::sendMessage_signal,
            this, &FWsClientPrivate::sendMessage_slot, Qt::QueuedConnection);
}

FWsClientPrivate::~FWsClientPrivate()
{
    mSocket->abort();
}

void FWsClientPrivate::checkAlive(bool on, int timeout_sec)
{
    if (on) {
        m_heartbeatCount = 0;
        m_heartbeatTimeout_sec = timeout_sec;
        m_keepAliveTimer->start();
        qInfo() << "checkAlive start, timeout:" << timeout_sec;
    } else {
        m_keepAliveTimer->stop();
    }
}

void FWsClientPrivate::startUdp(quint16 udp_port)
{
    for (int i = 0; i < 3; ++i) {
        quint16 port = udp_port + i;
        bool ok = mUdp->bind(port);
        if (ok) {
            qDebug() << "bind udp success. port:" << port;
            return;
        } else {
            qDebug() << "bind udp fail. port:" << port;
        }
    }
}

void FWsClientPrivate::onConnected_slot()
{
    qInfo().noquote() << QString("WsClient(%1)[%2(%3)] connected.").arg(m_clientName, m_url, m_serverName);

    m_isConnecting = false;
    m_isConnected = true;
    m_heartbeatCount = 0;

    m_reconnectTimer->stop();

    Q_Q(FWsClient);
    emit q->onConnected_signal();

    if (m_clientName.isEmpty() == false) {
        mSocket->sendTextMessage(QString("[ClientName]%1").arg(m_clientName));
    }
}

void FWsClientPrivate::onDisconnected_slot()
{
    if (m_verbose) {
        if (mSocket->closeCode() != QWebSocketProtocol::CloseCodeNormal) {
            qInfo().noquote() << QString("WsClient(%1)[%2(%3)] disconnected.").arg(m_clientName, m_url, m_serverName)
                              << mSocket->errorString() << mSocket->closeCode() << mSocket->closeReason();
        } else {
            qInfo().noquote() << QString("WsClient(%1)[%2(%3)] disconnected.").arg(m_clientName, m_url, m_serverName);
        }
    }

    m_isConnecting = false;
    m_isConnected = false;

    if (m_isAutoReconnect == true && m_reconnectTimer->isActive() == false) {
        m_reconnectTimer->start();
    }

    Q_Q(FWsClient);
    emit q->onDisConnected_signal();
}

void FWsClientPrivate::onReConnectedTimer_slot()
{
    if (m_isAutoReconnect && mSocket->isValid() == false && m_isConnected == false && m_isConnecting == false) {
        m_isConnecting = true;

        m_url = QString("ws://%1:%2").arg(m_address).arg(m_port);

        if (m_verbose) {
            qInfo().noquote() << QString("WsClient[%1 %2] reconnecting...").arg(m_serverName, m_url);
        }

        mSocket->open(QUrl(m_url));
    }
}

void FWsClientPrivate::onKeepAliveTimer_slot()
{
    if (m_isConnected) {
        mSocket->sendTextMessage(QStringLiteral("Alive"));
        bool ok = mSocket->flush();
        if (ok == false) {
            qWarning() << "send Alive false.";
        }

        m_heartbeatCount++;
        if (m_heartbeatCount > 1) {
            qWarning() << "heartbeat:" << m_heartbeatCount;
        }
        if (m_heartbeatCount > m_heartbeatTimeout_sec) {
            qWarning() << "heartbeat timeout, client abort!";
            mSocket->abort();
        }
    }
}

void FWsClientPrivate::onSslErrors_slot(const QList<QSslError> &errors)
{
    qWarning() << "[WsClient] SSL errors." << errors;

    QStringList errorList;
    for (const auto &error : errors) {
        errorList.append(error.errorString());
    }
    Q_Q(FWsClient);
    emit q->onError_signal(errorList);

    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root to the CA store.
    // mSocket.ignoreSslErrors();
}

void FWsClientPrivate::sendMessage_slot(const QString &message)
{
    /* SEND MESSAGE */
    if (m_isConnected) {
        mSocket->sendTextMessage(message);
        bool ok = mSocket->flush();
        if (ok) {
            if (m_verbose) {
                qDebug().noquote() << QString("[%1] <<").arg(m_serverName) << message.simplified();
            }
        } else {
            qWarning() << "[WsClient] sending failed.";
        }
    } else {
        qWarning() << "[WsClient] sending failed. mSocket is NOT Valid";
    }
}

void FWsClientPrivate::onReadDatagram_slot()
{
    while (mUdp->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mUdp->receiveDatagram();
        QString message(datagram.data());
        if (!message.isEmpty()) {
            qDebug() << "[UDP] >>" << message;

            if (message.startsWith("FWsServer")) {
                QString str = message.remove("FWsServer");

                QRegExp exp("\\[[\\w.:]+\\]");
                int pos = 0;
                QStringList list;
                while ((pos = exp.indexIn(str, pos)) != -1) {
                    pos += exp.matchedLength();
                    QString s = exp.cap();
                    s = s.mid(1, s.length() - 2);
                    list.append(s);
                }
                if (list.count() >= 2) {
                    QString key = list.at(0);
                    QString addr = list.at(1);
                    qDebug() << QString("key:%1, url:%2").arg(key, addr);

                    if (key == m_keyword) {
                        QStringList addrlist = addr.split(":");
                        if (addrlist.count() == 2) {
                            QString addr_ip = addrlist.at(0);
                            QString addr_port = addrlist.at(1);
                            int port = addr_port.toUInt();

                            if (!addr_ip.isEmpty() && port > 0) {
                                Q_Q(FWsClient);
                                emit q->onNewServerAddress_signal(addr_ip, port);
                            }
                        }
                    }
                }
            }
        }
    }
}

void FWsClientPrivate::onMessageReceived_slot(QString message)
{
    m_heartbeatCount = 0;

    if (message == QStringLiteral("Alive")) {
        return;
    }

    Q_Q(FWsClient);
    emit q->onReceiveMessage_signal(message);

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "[WsClient]: Json Parse Error" << jsonError.errorString() << "Raw Message:" << message;
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        if (m_verbose) {
            qDebug().noquote() << QString("[%1] >>").arg(m_serverName) << message.simplified();
        }

        /* GET MESSAGE */
        emit q->onReceiveMessageObj_signal(obj);

        JPacket p = FJsonRpcParser::decode(obj);
        emit q->onReceiveMessagePacket_signal(p);
    }
}

void FWsClientPrivate::autotestProcess(const JPacket &packet)
{
    if (packet.api == "AutoTest") {
        JPacket p(PacketType::Result, packet.id);
        p.resValue = true;
        QJsonObject obj = FJsonRpcParser::encode(p);
        QByteArray data = FJsonRpcParser::convertObjToByte(obj);

        QTimer::singleShot(2, this, [=](){
            sendMessage_slot(QString(data));
        });
    }
}
