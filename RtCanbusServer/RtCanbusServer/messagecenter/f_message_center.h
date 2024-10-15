#ifndef F_MESSAGE_CENTER_H
#define F_MESSAGE_CENTER_H

#include "f_jsonrpc_parser.h"

#include <QObject>
#include <QMap>

class FWebSocket;
class FMessageCenter : public QObject
{
    Q_OBJECT
public:
    static FMessageCenter *GetInstance();

    void sendMessage(const QString &client_id, JPacket &p, const QString &dev_id);
    void sendNotification(JPacket &p, const QString &dev_id);

signals:
    void onConnected_signal(const QString &client_id);
    void onDisconnected_signal(const QString &client_id);

    void onReceiveMessage_signal(const QString &client_id, const QString &message);
    void onReceiveJsonObj_signal(const QString &client_id, const QString &dev_id, const QJsonObject &obj);
    void onReceivePacket_signal(const QString &client_id, const QString &dev_id, const JPacket &p);

private:
    explicit FMessageCenter(QObject *parent = nullptr);
    Q_DISABLE_COPY(FMessageCenter)

    FWebSocket *m_socket;
    QMap<QString, QString> mClientMap;

private slots:
    void handleWSmessage_slot(const QString &client_id, const QJsonObject &msgObj);
};

#endif // F_MESSAGE_CENTER_H
