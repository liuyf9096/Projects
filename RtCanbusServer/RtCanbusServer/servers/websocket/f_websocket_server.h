#ifndef F_WS_SERVER_H
#define F_WS_SERVER_H

#include "messagecenter/f_jsonrpc_parser.h"

#include <QObject>
#include <QJsonObject>

class FWebSocketServerPrivate;
class FWebSocketServer : public QObject
{
    Q_OBJECT
public:
    static FWebSocketServer *GetInstance();

    void listen(quint16 port, const QString &udpKey = QString());
    void closeServer();

    void setClientUserName(const QString &client_id, const QString &username);
    void checkAlive(bool on, int timeout_sec = 10);

    /* Send Message To Client, Recommend : sendPacket */
    void sendMessage(const QString &client_id, const QString &message);
    void sendJsonObject(const QString &client_id, const QJsonObject &obj);
    void sendPacket(const QString &client_id, const JPacket &p);

    /* default: Rx_Port:20020 Tx_Port:20010~20012 */
    void setUdpPort(quint16 txPort);

    void setSendingExemptWords(const QStringList &list);
    void setReceivingExemptWords(const QStringList &list);

signals:
    /* Server Listen Status */
    void onListenState_signal();
    void onListenStateFail_signal();

    /* Client Status */
    void onClientConnected_signal(const QString &client_id);
    void onClientDisconnected_signal(const QString &client_id);
    void onDetectNewDevice_signal(const QString &client_id, const QString &client_name);

    /* Receive Message From Client */
    void onReceiveMessage_signal(const QString &client_id, const QString &message);
    void onReceiveMessageJsonObj_signal(const QString &client_id, const QJsonObject &obj);

    void onSendMsgError_signal(const QString &client_id, const QString &message);

private:
    explicit FWebSocketServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(FWebSocketServer)

    FWebSocketServerPrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FWebSocketServer)
};

#endif // F_WS_SERVER_H
