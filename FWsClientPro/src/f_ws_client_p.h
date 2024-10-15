#ifndef F_WS_CLIENT_P_H
#define F_WS_CLIENT_P_H

#include <QObject>
#include <QWebSocket>

class JPacket;
class QTimer;
class QUdpSocket;
class FWsClient;
class FWsClientPrivate : public QObject
{
    Q_OBJECT
public:
    FWsClientPrivate(FWsClient *parent);
    virtual ~FWsClientPrivate();

    void checkAlive(bool on, int timeout_sec);
    void startUdp(quint16 udp_port);

signals:
    void sendMessage_signal(const QString &message);

private:
    FWsClient * const q_ptr;
    Q_DECLARE_PUBLIC(FWsClient)

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

    void autotestProcess(const JPacket &packet);

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
};
#endif // F_WS_CLIENT_P_H
