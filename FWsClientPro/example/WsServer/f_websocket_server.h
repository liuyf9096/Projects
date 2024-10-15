#ifndef F_WEBSOCKET_SERVER_H
#define F_WEBSOCKET_SERVER_H

#include <QObject>
#include <QJsonObject>

class FWebSocketServerPrivate;
class FWebSocketServer : public QObject
{
    Q_OBJECT
public:
    static FWebSocketServer *GetInstance();

    void listenPort(quint16 port);

    void closeServer();

    void sendMessage(const QString &client_id, const QString &message);
    void setDeviceName(const QString &client_id, const QString &deviceName);

signals:
    void receiveMassage_signal(const QString &client_id, const QString &message);

    void onClientConnected_signal(const QString &client_id);
    void onClientDisconnected_signal(const QString &client_id);

private:
    explicit FWebSocketServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(FWebSocketServer)

    FWebSocketServerPrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FWebSocketServer)
};

#endif // F_WEBSOCKET_SERVER_H
