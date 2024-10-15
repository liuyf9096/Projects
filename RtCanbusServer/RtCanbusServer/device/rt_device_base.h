#ifndef R_DEVICE_BASE_H
#define R_DEVICE_BASE_H

#include "messagecenter/f_message_center.h"

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QTimer>
#include <QBitArray>
#include <QDebug>

enum ReplyType {
    CommuType_None = 0,
    CommuType_ACK = 1,
    CommuType_Done_OK = 2,
    CommuType_Done_Fail = 3,
    CommuType_Command_Error = 4,
    CommuType_Passive = 5,
    CommuType_Notification = 6
};

class CanReqPacket {
public:
    CanReqPacket();
    CanReqPacket(quint16 can_id, const QString &api, quint16 cmd_u, const QByteArray &data = QByteArray());

    quint16 canId;
    QString api;
    quint16 cmdId;
    QString subapi;
    QString clientId;

    QByteArray data;
};

class WsRequest {
public:
    WsRequest();
    WsRequest(const QString &clientid, quint64 wsid, quint16 canid, const JPacket &p);

    QString client_id;
    quint64 ws_id;
    quint16 can_id;
    JPacket packet;
};

class FSqlDatabase;
class RtDeviceBase : public QObject
{
    Q_OBJECT
public:
    explicit RtDeviceBase(const QString &dev_id, quint16 address, const QString &type, QObject *parent = nullptr);

    static QStringList ReplyType;

    /* basic */
    QString type() { return m_deviceType; }
    QString deviceID() { return m_devID; }
    quint16 getAddress() { return m_address; }

    void setUsername(const QString &username);
    QString username() { return m_username; }

    void setSendMsgInterval(int interval);
    void setCheckSensorInterval(int interval);

    /* sensor */
    void setCheckSensorEnable(const QString &client_id, bool en);
    void setCheckSensorStart();
    void setCheckSensorStop();
    void setSensors(const QJsonArray &arr);
    void setFloaterSensors(const QJsonArray &arr);

    void setCheckFloaterSensorEnable(const QString &client_id, bool en, int count);

    quint16 cmd_GetAllSensorValue();
    quint16 cmd_GetAllFloaterSensorValue(int count);

signals:
    void sendCanMessage_signal(quint16 canid, quint16 destID, quint16 module, quint16 cmd,
                               const QByteArray &params, const QString &userinfo);

public slots:
    void handleWSMessage_slot(const QString &client_id, const QString &dev_id, const JPacket &p);
    void handleCanbusMessage_slot(int address, const QJsonObject &obj, const QByteArray &resValue);

protected:
    virtual bool handlePacket(quint16 &can_id, const JPacket &p) = 0;
    virtual QJsonValue handleResult(const WsRequest &req, const QByteArray &array) = 0;
    virtual QJsonValue handleSelfResult(const QString &api, const QByteArray &array) = 0;

    /* canbus result */
    void handleReceiveCanAck(quint16 canid, const QString &api, const QJsonObject &resObj);
    bool handleReceiveCanResult(quint16 canid, const QString &api, const QJsonObject &resObj, const QByteArray &data);
    void handleReceiveResultCanFail(quint16 canid, const QString &api, const QJsonObject &resObj);
    void handleReceiveResultCanError(quint16 canid, const QString &api, const QJsonObject &resObj);
    void handleCanbusNotification(const QJsonObject &obj, const QByteArray &resValue);

    /* Sensor Status */
    void handleAllSensorData(quint16 canid, const QBitArray &bits);
    QMap<QString, bool> m_sensorValueMap;
    QMap<int, QString> SensorsMap;
    QMap<int, QString> FloaterSensorsMap;

    /* Floater Sensor */
    QString m_queryClientId;
    void handleAllFloaterSensorData(quint16 canid, QByteArray data);

    /* Command Map */
    QMap<QString, int> mComboActionMap;

    quint16 newId();
    QList<CanReqPacket> m_sendingList;

private slots:
    void _sendingCanPacket_slot();
    void _checkAllSensors_slot();
    void _checkFloaterSensor_slot();

private:
    const QString m_deviceType;
    const QString m_devID;
    const quint16 m_address;

    QString m_username;
    quint16 m_can_id;    /* 0~65535 */
    QString m_sqlHistoryTable;

    /* canid <-> clientid, wsid, packet */
    QMap<quint16, QString> m_canid_clientid_Map;
    QMap<quint16, WsRequest> m_canid_wsReq_Map;

    /* canid -> canbus */
    QMap<quint16, CanReqPacket> m_canReqMap;

    /* websocket */
    FMessageCenter *g_ws;

    /* LOOP: send message to canbus */
    QTimer *m_sendingTimer;

    /* check sensors */
    QMap<QString, bool> m_client_sensorCheck_Map;
    QTimer *m_checkSensorTimer;
    bool m_sensorUpdate;
    int timeoutCount;
    quint16 m_lastSensorId;
    int m_maxport;

    /* floater */
    QTimer *m_checkFloaterSensorTimer;
    int m_floaterCount;

    FSqlDatabase *mDb;

private:
    void init();

    /* sensors */
    void send_ResultOK(const QString &client_id, quint64 id);

    /* sql */
    void insertRecordDB(quint64 ws_id, quint16 can_id, const QString &api);
    void updateRecordState(quint64 ws_id, int stateCode);
    void updateRecordAckTime(quint64 ws_id, const QString &time);
    void updateRecordStopTime(quint64 ws_id, const QString &time);
    void updateRecordDurationTime(quint64 ws_id);
};

#endif // R_DEVICE_BASE_H

