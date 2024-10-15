#ifndef F_WS_CLIENT_H
#define F_WS_CLIENT_H

/************************************************************
 * function : used for communication with websocket server  *
 *   author : liuyufei                                      *
 * datetime : 2021-04-12 09:22:41                           *
 ************************************************************/

#if defined(FWSCLINET_LIBRARY)
#  define FWSCLINET_EXPORT Q_DECL_EXPORT
#else
#  define FWSCLINET_EXPORT Q_DECL_IMPORT
#endif

#include <QtGlobal>
#include <QObject>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

class JPacket;
class FWsClientPrivate;
class FWSCLINET_EXPORT FWsClient : public QObject
{
    Q_OBJECT
public:
    explicit FWsClient(QObject *parent = nullptr);

    bool isConnected();
    QString address();
    quint16 port();

    /* set address and port */
    void setServerAddress(const QString &address);
    void setServerWSPort(quint16 port);
    void setServerAddressPort(const QString &address, quint16 port);

    /* basic info */
    void setClientName(const QString &name);
    void autoDetectServer(const QString &keyword, quint16 udp_port = 20010);

    /* connect to the server, such as "ws://localhost:9090" */
    Q_INVOKABLE void connectServer(bool isAutoReconnect = true, const QString &serverName = QString());
    Q_INVOKABLE void connectServer(const QString &address, quint16 port, bool isAutoReconnect = true, const QString &serverName = QString());
    Q_INVOKABLE void disconnectServer();

    /* set auto reconnect interval, default: 1000ms */
    void setAutoReconnectInterval(int interval);

    /* check heart beat */
    void checkAlive(bool on, int timeout_sec = 10);

    /* three ways to send message to the connected server */
    void sendMessage(const QString &message);
    void sendMessage(const QJsonObject &obj);
    void sendMessage(const JPacket &packet);

    /* debug: print message info, default:on */
    void setVerboseOn(bool on);

signals:
    /* update server address */
    void onNewServerAddress_signal(const QString &address, quint16 port);

    /* received message */
    void onReceiveMessage_signal(const QString &message);
    void onReceiveMessageObj_signal(const QJsonObject &obj);
    void onReceiveMessagePacket_signal(const JPacket &packet);

    /* connection status */
    void onConnected_signal();
    void onDisConnected_signal();
    void onError_signal(const QStringList &error);

private:
    FWsClientPrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FWsClient)
};

#endif // F_WS_CLIENT_H
