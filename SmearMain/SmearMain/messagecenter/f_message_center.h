#ifndef F_MESSAGE_CENTER_H
#define F_MESSAGE_CENTER_H

#include "f_jsonrpc_parser.h"

#include <QObject>
#include <QMap>

enum Commun_Destination {
    Commun_UI,
    Commun_United,
    Commun_Reader
};

class FWebSocket;
class FMessageCenter : public QObject
{
    Q_OBJECT
public:
    static FMessageCenter *GetInstance();

    FWebSocket *mCanbus;
    FWebSocket *mUnity;
    FWebSocket *mReader;

    bool isUIConnected();
    bool isUnityConnected();
    bool isCanbusConnected();
    bool isReaderConnected();

    quint64 sendCanbusMessage(JPacket &request, const QString &username = QString());
    quint64 sendUIMessage(JPacket &packet);
    quint64 sendUnityMessage(JPacket &packet);
    quint64 sendReaderMessage(JPacket &packet);

    void sendDoneResultMessage(Commun_Destination des, quint64 id);

signals:
    void onConnected_signal(const QString &address, const QString &username = QString());
    void onDisconnected_signal(const QString &address, const QString &username = QString());

    /* canbus */
    void onReceiveCanbusPacket_signal(const JPacket &result, const JPacket &request);
    void onReceiveCanbusNotification_signal(const JPacket &notification);
    void gripperStatusInfo_signal(const QJsonObject &obj);

    /* ui */
    void onUIMessageRequestPacket_signal(const JPacket &request);
    void onUIMessageResultPacket_signal(const JPacket &result, const JPacket &request);

    /* unity */
    void onUnityMessageResult_signal(const JPacket &result, const JPacket &request);

    /* reader */
    void onReaderMessageResult_signal(const JPacket &result, const JPacket &request);

public slots:
    void onReceiveUIMessage_slot(const QString &client_id, const QJsonObject &msgObj);
    void onReceiveCanbusMessage_slot(const QJsonObject &obj);
    void onReceiveUnityMessage_slot(const QJsonObject &obj);
    void onNewUnityServerAddress_slot(const QString &address, quint16 port);

    /* reader */
    void onReceiveReaderMessage_slot(const QJsonObject &obj);
    void onNewReaderServerAddress_slot(const QString &address, quint16 port);

private:
    explicit FMessageCenter(QObject *parent = nullptr);
    Q_DISABLE_COPY(FMessageCenter)

    quint64 m_canbus_id;
    QMap<quint64, JPacket> m_CanbusPacketMap;

    quint64 m_ws_id;
    QMap<quint64, JPacket> m_WsPacketMap;

    QString mUI_clientid;

    bool m_isUIConnected;
    bool m_isCanbusConnected;

    bool handleKeyNotification(JPacket &packet);
    void handleCanbusException(JPacket &packet);

    void sendReaderConnectionStatus(bool on);
    void sendReaderConnectionStatus();

    void configModule(JPacket &packet);
};

#endif // F_MESSAGE_CENTER_H
