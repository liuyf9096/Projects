#ifndef F_TCPSOCKET_SERVER_P_H
#define F_TCPSOCKET_SERVER_P_H

#include <QObject>
#include <QMap>

class QThread;
class QTcpServer;
class QTcpSocket;
class FTcpSocketServer;
class FTcpSocketServerPrivate : public QObject
{
    Q_OBJECT
public:
    FTcpSocketServerPrivate(FTcpSocketServer *parent);
    virtual ~FTcpSocketServerPrivate();

    Q_INVOKABLE void listen(quint16 port);
    Q_INVOKABLE void setClientName(const QString dev_id, const QString userName);
    Q_INVOKABLE void closeAllClinets();
    Q_INVOKABLE void stopServer();

signals:
    void sendMessage_signal(const QString &dev_id, const QString &message);

private:
    FTcpSocketServer * const q_ptr;
    Q_DISABLE_COPY(FTcpSocketServerPrivate)
    Q_DECLARE_PUBLIC(FTcpSocketServer)

    QThread *mThread;
    QTcpServer *mServer;
    QMap<QString, QTcpSocket *> m_clientMap;

private slots:
    void sendMessage_slot(const QString &dev_id, const QString &message);

    //![TCPSocket Server]
    void onNewConnection_slot();

    //![Client]
    void onClientMessageReceived_slot();
    void onClientDisconnection_slot();
};

#endif // F_TCPSOCKET_SERVER_P_H
