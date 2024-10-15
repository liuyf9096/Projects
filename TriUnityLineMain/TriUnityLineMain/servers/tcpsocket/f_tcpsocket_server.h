#ifndef F_TCPSOCKET_SERVER_H
#define F_TCPSOCKET_SERVER_H

#include <QObject>
#include <QJsonObject>

class FTcpSocketServerPrivate;
class FTcpSocketServer : public QObject
{
    Q_OBJECT
public:
    static FTcpSocketServer *GetInstance();

    void sendMessage(const QString &dev_id, const QString &message);
    void setDeviceName(const QString &dev_id, const QString &deviceName);
    void closeServer();

signals:
    void onClientConnected_signal(const QString &dev_id);
    void receiveMassage_signal(const QString &dev_id, const QString &message);
    void onClientDisconnected_signal(const QString &dev_id);

private:
    explicit FTcpSocketServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(FTcpSocketServer)

    FTcpSocketServerPrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FTcpSocketServer)
};

#endif // F_TCPSOCKET_SERVER_H
