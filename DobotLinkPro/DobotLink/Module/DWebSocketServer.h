#ifndef DWEBSOCKETSERVER_H
#define DWEBSOCKETSERVER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class QWebSocket;
class QWebSocketServer;
class DWebSocketServer : public QObject
{
    Q_OBJECT
public:
    static DWebSocketServer *getInstance();

    void sendMessage(quint16 port, QString message);
    void sendMessageObj(QJsonObject obj);
    void closeServer();

signals:
    void receiveMassage_signal(quint16 port, QString message);
    void clientClose_signal(quint16 port);

private:
    Q_DISABLE_COPY(DWebSocketServer)
    explicit DWebSocketServer(QObject *parent = nullptr);

    QWebSocketServer *m_WebSocketServer;
    QMap<quint16, QWebSocket *> m_clientMap;

private slots:
//![WebSocketServer]
    void onNewConnection_slot();
    void onWebSocketServerClosed_slot();

//![WebSocketClient]
    void onClientMessageReceived_slot(QString message);
    void onClientDisconnection_slot();
};

#endif // DWEBSOCKETSERVER_H
