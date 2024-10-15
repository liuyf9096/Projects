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

    void addStaionIp(const QString &station, const QString &ip) { mStationIpMap.insert(station, ip); }

    FWebSocket *mCanbus;
    FWebSocket *mDMU;

    bool isDMUConnected();
    bool isCanbusConnected();
    bool isUnitedUIConnected();

    quint64 sendClientMessage(const QString &client_id, JPacket &packet);

    quint64 sendCanbusMessage(JPacket &packet, const QString &username = QString());
    quint64 sendUIMessage(JPacket &packet);
    quint64 sendDMUMessage(JPacket &packet);

signals:
    void onClientConnected_signal(const QString &client_id, bool connected);
    void onClientDeviceConnected_signal(const QString &client_dev, const QString &client_id, bool connected);

    /* canbus */
    void onReceiveCanbusPacket_signal(const JPacket &result, const JPacket &request);
    void onReceiveCanbusNotification_signal(const JPacket &notification);

    /* DMU */
    void onDMUMessageRequestPacket_signal(const JPacket &request);
    void onDMUMessageResultPacket_signal(const JPacket &result, const JPacket &request);

    /* clients */
    void onClientsMessageNotification_signal(const QString &client_id, const JPacket &note);
    void onClientsMessageRequestPacket_signal(const QString &client_id, const JPacket &request);
    void onClientsMessageResultPacket_signal(const QString &client_id, const JPacket &result, const JPacket &request);
    void onClientsMessageErrorPacket_signal(const QString &client_id, const JPacket &error, const JPacket &request);

public slots:
    void onReceiveClientsMessage_slot(const QString &client_id, const QJsonObject &obj);
    void onReceiveCanbusMessage_slot(const QJsonObject &obj);
    void onReceiveDMUMessage_slot(const QJsonObject &obj);

    void onCanbusConnected_slot();

    void onDMUConnected_slot();
    void onDMUDisonnected_slot();
    void onDMUNewServerAddress_slot(const QString &address, quint16 port);

private:
    explicit FMessageCenter(QObject *parent = nullptr);
    Q_DISABLE_COPY(FMessageCenter)

    QJsonObject m_versionObj;

    QMap<QString, QString> mStationIpMap;

    quint64 m_canbus_id;
    QMap<quint64, JPacket> m_CanbusPacketMap;

    quint64 m_ws_id;
    QMap<quint64, JPacket> m_WsPacketMap;

    bool m_isUIConnected;
    QString m_ui_client_id;

    void handleKeyNotification(const QJsonObject &obj);
    void handleCanbusException(JPacket &packet);
    void handleUIpacket(const JPacket &p);
    void handleSystemDateTime(const QJsonObject &obj);
    void handleCanbusVersion(const JPacket &result, const JPacket &request);
};

#endif // F_MESSAGE_CENTER_H
