#ifndef F_WS_SERVER_P_H
#define F_WS_SERVER_P_H

#include <QObject>
#include <QMap>

class QTimer;
class QThread;
class QUdpSocket;
class QWebSocket;
class QWebSocketServer;
class FWebSocketServer;
class FWebSocketServerPrivate : public QObject
{
    Q_OBJECT
public:
    FWebSocketServerPrivate(FWebSocketServer *parent);
    virtual ~FWebSocketServerPrivate();

    Q_INVOKABLE void listen(quint16 port, const QString &udpKey);
    Q_INVOKABLE void setClientUserName(const QString client_id, const QString userName);
    Q_INVOKABLE void closeAllClinets();
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void checkAlive(bool on, int timeout_sec);

    void sendUdpMessage(const QString &message);

signals:
    void sendMessage_signal(const QString &client_id, const QString &message);
    void onSendMsgError_signal(const QString &client_id, const QString &message);

private:
    FWebSocketServer * const q_ptr;
    Q_DECLARE_PUBLIC(FWebSocketServer)

    QString m_address;
    QString m_serverName;

    quint16 m_listenPort;
    QString m_udpKeyword;

    QThread *mThread;
    QWebSocketServer *mServer;
    QUdpSocket *mUdp;
    quint16 m_sendPort;
    QMap<QString, QWebSocket *> m_clientMap;

    quint16 m_newPort;
    QString n_newUdpKey;

    QTimer *m_heartbeatTimer;
    int m_heartbeatTimeout_sec;

    QStringList m_sendingExemptList;
    QStringList m_receivingExemptList;
    bool checkSendingExmptWords(const QString &message);
    bool checkReceivingExmptWords(const QString &message);

private slots:
    void sendMessage_slot(const QString &client_id, const QString &message);

    //![WebSocket Server]
    void onNewConnection_slot();
    void onWebSocketServerClosed_slot();

    //![Client]
    void onClientMessageReceived_slot(const QString &message);
    void onClientDisconnection_slot();

    //![UDP]
    void onReadDatagram_slot();

    void heartbeatTimer_slot();
};

#endif // F_WS_SERVER_P_H
