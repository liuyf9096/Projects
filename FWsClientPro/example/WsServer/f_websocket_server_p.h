#ifndef F_WEBSOCKET_SERVER_P_H
#define F_WEBSOCKET_SERVER_P_H

#include <QObject>
#include <QMap>

class QThread;
class QWebSocket;
class QWebSocketServer;
class FWebSocketServer;
class FWebSocketServerPrivate : public QObject
{
    Q_OBJECT
public:
    FWebSocketServerPrivate(FWebSocketServer *parent);
    virtual ~FWebSocketServerPrivate();

    Q_INVOKABLE void listen(quint16 port);
    Q_INVOKABLE void setClientName(const QString client_id, const QString userName);
    Q_INVOKABLE void closeAllClinets();
    Q_INVOKABLE void stopServer();

signals:
    void sendMessage_signal(const QString &client_id, const QString &message);

private:
    FWebSocketServer * const q_ptr;
    Q_DISABLE_COPY(FWebSocketServerPrivate)
    Q_DECLARE_PUBLIC(FWebSocketServer)

    QString m_address;

    QThread *mThread;
    QWebSocketServer *mServer;
    QMap<QString, QWebSocket *> m_clientMap;

private slots:
    void sendMessage_slot(const QString &client_id, const QString &message);

    //![WebSocket Server]
    void onNewConnection_slot();
    void onWebSocketServerClosed_slot();

    //![Client]
    void onClientMessageReceived_slot(const QString &message);
    void onClientDisconnection_slot();
};



#endif // F_WEBSOCKET_SERVER_P_H
