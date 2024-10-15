#include "f_websocket.h"

#include <QWebSocket>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QNetworkDatagram>
#include <QSslError>
#include <QNetworkProxy>
#include <QUrl>
#include <QTimer>
#include <QDebug>

static const int DEFAULT_RECONNECT_MS = 1000;

FWebSocket::FWebSocket(QObject *parent)
    : QObject(parent)
    , m_verbose(true)
    , m_isConnecting(false)
    , m_isConnected(false)
    , m_isAutoReconnect(false)
    , m_heartbeatCount(0)
{
    m_heartbeatTimeout_sec = 10;

    QNetworkProxyFactory::setUseSystemConfiguration(false);

    mSocket = new QWebSocket();
    mSocket->setParent(this);
    mSocket->setProxy(QNetworkProxy::NoProxy);
    connect(mSocket, &QWebSocket::connected, this, &FWebSocket::onConnected_slot);
    connect(mSocket, &QWebSocket::disconnected, this, &FWebSocket::onDisconnected_slot);
    connect(mSocket, &QWebSocket::aboutToClose, [=](){
        qWarning() << "aboutToClose:" << mSocket->errorString() << mSocket->closeCode() << mSocket->closeReason();
    });
    connect(mSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &FWebSocket::onSslErrors_slot);
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            [=](QAbstractSocket::SocketError error){
        qWarning() << "WsClient Error:" << error;
    });
    connect(mSocket, &QWebSocket::textMessageReceived, this, &FWebSocket::onMessageReceived_slot);

    mUdp = new QUdpSocket(this);
    connect(mUdp, &QUdpSocket::readyRead,
            this, &FWebSocket::onReadDatagram_slot);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(DEFAULT_RECONNECT_MS);
    connect(m_reconnectTimer, &QTimer::timeout, this, &FWebSocket::onReConnectedTimer_slot);

    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(1000);
    connect(m_keepAliveTimer, &QTimer::timeout, this, &FWebSocket::onKeepAliveTimer_slot);

    qDebug().noquote() << QString("FWsClient created. version:%1").arg("2.1.6");
}

bool FWebSocket::isConnected()
{
    return m_isConnected;
}

QString FWebSocket::address()
{
    return m_address;
}

quint16 FWebSocket::port()
{
    return m_port;
}

void FWebSocket::setServerAddress(const QString &address)
{
    m_address = address;
    m_url = QString("ws://%1:%2").arg(address).arg(m_port);
}

void FWebSocket::setServerWSPort(quint16 port)
{
    m_port = port;
    m_url = QString("ws://%1:%2").arg(m_address).arg(port);
}

void FWebSocket::setServerAddressPort(const QString &address, quint16 port)
{
    m_address = address;
    m_port = port;
    m_url = QString("ws://%1:%2").arg(address).arg(port);
}

void FWebSocket::setClientName(const QString &name)
{
    m_clientName = name;
}

void FWebSocket::autoDetectServer(const QString &udpkey, quint16 udp_port)
{
    m_keyword = udpkey;

    for (int i = 0; i < 3; ++i) {
        quint16 port = udp_port + i;
        bool ok = mUdp->bind(port);
        if (ok) {
            qDebug() << "bind udp OK. port:" << port;
            break;
        } else {
            qDebug() << "bind udp Fail. port:" << port;
        }
    }
}

void FWebSocket::connectServer(bool isAutoReconnect, const QString &serverName)
{
    connectServer(m_address, m_port, isAutoReconnect, serverName);
}

void FWebSocket::connectServer(const QString &address, quint16 port, bool isAutoReconnect, const QString &serverName)
{
    m_serverName = serverName;

    if (m_isConnecting == false) {
        m_isConnecting = true;
        if (m_isConnected == true) {
            if (m_address == address && m_port == port) {
                qInfo() << "WsClient is already connected.";
            } else {
                qWarning() << "WsClient: please disconnect the existing connection first.";
            }
        } else {
            m_address = address;
            m_port = port;
            m_url = QString("ws://%1:%2").arg(address).arg(port);
            m_isAutoReconnect = isAutoReconnect;

            if (m_verbose) {
                qInfo() << "WsClient is connecting to server:" << serverName << m_url;
            }

            /* connect url */
            mSocket->open(QUrl(m_url));
        }
    } else {
        qWarning() << "WsClient: It's connecting, please disconnectServer first.";
    }
}

void FWebSocket::disconnectServer()
{
    m_isConnecting = false;
    m_isConnected = false;
    mSocket->abort();
    m_reconnectTimer->stop();
}

void FWebSocket::setAutoReconnectInterval(int interval)
{
    if (interval < 20) {
        qWarning() << "WsClient auto reconnect timer interval is too small." << QString("%1ms").arg(interval);
    } else {
        m_reconnectTimer->setInterval(interval);
        qInfo() << "WsClient: set client auto reconnect timer interval:" << QString("%1ms").arg(interval);
    }
}

void FWebSocket::checkAlive(bool on, int timeout_sec)
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

void FWebSocket::setVerboseOn(bool on)
{
    m_verbose = on;
}

void FWebSocket::sendMessage(const QString &message)
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

void FWebSocket::sendMessage(const JPacket &p)
{
    QJsonObject obj = FJsonRpcParser::encode(p);
    sendMessage(obj);
}

void FWebSocket::sendMessage(const QJsonObject &obj)
{
    QByteArray data = FJsonRpcParser::convertObjToByte(obj);
    sendMessage(data);
}

/* SLOT */
void FWebSocket::onConnected_slot()
{
    qInfo().noquote() << QString("WsClient(%1)[%2(%3)] connected.").arg(m_clientName, m_url, m_serverName);

    m_isConnecting = false;
    m_isConnected = true;
    m_heartbeatCount = 0;

    m_reconnectTimer->stop();

    if (m_clientName.isEmpty() == false) {
        QString str = QString("[ClientName]%1").arg(m_clientName);
        qDebug().noquote() << QString("[%1] <<").arg(m_serverName) << str;
        mSocket->sendTextMessage(str);
    }

    emit onConnected_signal();
}

void FWebSocket::onDisconnected_slot()
{
    if (m_verbose) {
        qInfo().noquote() << QString("WsClient(%1)[%2(%3)] connected.").arg(m_clientName, m_url, m_serverName)
                          << mSocket->errorString() << mSocket->closeCode() << mSocket->closeReason();
    }

    m_isConnecting = false;
    m_isConnected = false;

    if (m_isAutoReconnect == true) {
        m_reconnectTimer->start();
    }

    emit onDisconnected_signal();
}

void FWebSocket::onReConnectedTimer_slot()
{
    if (mSocket->isValid() == false && m_isConnected == false && m_isConnecting == false) {
        m_isConnecting = true;

        m_url = QString("ws://%1:%2").arg(m_address).arg(m_port);

        if (m_verbose) {
            qInfo().noquote() << QString("WsClient[%1 %2] reconnecting...").arg(m_serverName, m_url);
        }

        mSocket->open(QUrl(m_url));
    }
}

void FWebSocket::onKeepAliveTimer_slot()
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

void FWebSocket::onMessageReceived_slot(QString message)
{
    m_heartbeatCount = 0;

    if (message == QStringLiteral("Alive")) {
        return;
    }

#if 0
    qDebug().noquote() << QString("[%1] >>").arg(m_serverName) << message.simplified();
#endif

    emit onReceiveMessage_signal(message);

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "[WsClient]: Json Parse Error" << jsonError.errorString() << "Raw Message:" << message;
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        /* GET MESSAGE */
        emit onReceiveMessageObj_signal(obj);

        JPacket p = FJsonRpcParser::decode(obj);

        if (m_verbose) {
            if (p.type != PacketType::Notification) {
                qDebug().noquote() << QString("[%1] >>").arg(m_serverName) << message.simplified();
            }
        }

        emit onReceiveMessagePacket_signal(p);
    }
}

void FWebSocket::onSslErrors_slot(const QList<QSslError> &errors)
{
    qWarning() << "[WsClient] SSL errors." << errors;

    QStringList errorList;
    for (const auto &error : errors) {
        errorList.append(error.errorString());
    }
    emit onError_signal(errorList);

    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root to the CA store.
    // mSocket.ignoreSslErrors();
}

void FWebSocket::sendMessage_slot(const QString &message)
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

void FWebSocket::onReadDatagram_slot()
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
                                emit onNewServerAddress_signal(addr_ip, port);
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Function */
QString FWebSocket::checkIpAddress(const QString &address)
{
    QRegExp rx("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
    if (rx.indexIn(address) > -1) {
        QString ip = rx.cap();
        return ip;
    }
    return QString();
}

bool FWebSocket::isIpAddress(const QString &address)
{
    if (address.isEmpty()) {
        return false;
    }

    QRegExp rx("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
    if (rx.indexIn(address) > -1) {
        return true;
    }
    return false;
}

