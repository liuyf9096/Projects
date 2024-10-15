#ifndef F_WEBSOCKET_H
#define F_WEBSOCKET_H

#include "messagecenter/f_jsonrpc_parser.h"

#include <QObject>
#include <QJsonObject>

class QWebSocket;
class QUdpSocket;
class QSslError;
class QTimer;
class FWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit FWebSocket(QObject *parent = nullptr);

    bool isConnected();
    QString address();
    quint16 port();

    /* set address and port */
    void setServerAddress(const QString &address);
    void setServerWSPort(quint16 port);
    void setServerAddressPort(const QString &address, quint16 port);

    /* basic info */
    void setClientName(const QString &name);
    void autoDetectServer(const QString &udpkey, quint16 udp_port = 20010);

    /* connect to the server, such as "ws://localhost:9090" */
    void connectServer(bool isAutoReconnect = true, const QString &serverName = QString());
    void connectServer(const QString &address, quint16 port, bool isAutoReconnect = true, const QString &serverName = QString());
    void disconnectServer();

    /* set auto reconnect interval, default: 1000ms */
    void setAutoReconnectInterval(int interval);

    /* check heart beat */
    void checkAlive(bool on, int timeout_sec = 10);

    /* debug: print message info, default:on */
    void setVerboseOn(bool on);

public slots:
    void sendMessage(const QString &message);
    void sendMessage(const JPacket &p);
    void sendMessage(const QJsonObject &obj);

signals:
    /* update server address */
    void onNewServerAddress_signal(const QString &address, quint16 port);

    /* received message */
    void onReceiveMessage_signal(const QString &message);
    void onReceiveMessageObj_signal(const QJsonObject &obj);
    void onReceiveMessagePacket_signal(const JPacket &packet);

    /* connection status */
    void onConnected_signal();
    void onDisconnected_signal();
    void onError_signal(const QStringList &error);

private:
    quint16 m_port;
    QString m_address;
    QString m_url;
    QString m_keyword;
    QString m_serverName;
    QString m_clientName;

    bool m_verbose;
    bool m_isConnecting;
    bool m_isConnected;
    bool m_isAutoReconnect;

    QWebSocket *mSocket;
    QUdpSocket *mUdp;
    QTimer *m_reconnectTimer;

    /* keep alive */
    QTimer *m_keepAliveTimer;
    int m_heartbeatCount;
    int m_heartbeatTimeout_sec;

private slots:
    void onConnected_slot();
    void onDisconnected_slot();

    void onReConnectedTimer_slot();
    void onKeepAliveTimer_slot();

    void onMessageReceived_slot(QString message);
    void onSslErrors_slot(const QList<QSslError> &errors);

    void sendMessage_slot(const QString &message);

    //![UDP]
    void onReadDatagram_slot();

private:
    QString checkIpAddress(const QString &address);
    bool isIpAddress(const QString &address);
};

#endif // F_WEBSOCKET_H
