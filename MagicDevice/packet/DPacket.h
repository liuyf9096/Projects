#ifndef DPACKET_H
#define DPACKET_H

#include <QObject>

class QByteArray;
class QJsonObject;

/* Ctrl: [target isQueue rw] */
class PacketCtrl
{
public:
    quint8 c_rw;
    bool c_isQueued;
    quint8 c_targetType;
    quint8 c_slaveid;

    PacketCtrl();

    void setPacketCtrl(const quint8 ctrl);
    void clear();

    quint8 getPacketCtrl();
    QString getTargetType();
    QString getRWType();
    int getSlaveID();
};


/* Payload: [cmdID Ctrl Params] */
class PacketPayload
{
public:
    quint8 p_cmdID;
    PacketCtrl p_ctrl;
    QByteArray p_params;

    PacketPayload();
    void setPacketPayload(QByteArray payload);
    QByteArray getPacketPayload();

    void clear();
};


/* Packet: [ head1 head2 length payload checksum ] */
class DPacket
{
public:
    enum PacketParseError {
        PARSE_NO_ERROR,
        PARSE_LENGTH_LESS_THAN_5,
        PARSE_LENGTH_NOT_ENOUGH,
        PARSE_HEAD_ERROR,
        PARSE_CHECKSUM_ERROR
    };

    explicit DPacket(quint64 id = 0);

    bool isEmpty();
    void clear();

    void setPacketID(quint64 id);
    quint64 getPacketID();

    int setPacket(QByteArray data);

    /* bool setPacket(QJsonObject data);
     * cmd: string,
     * isQueue: bool,
     * targetType: string ("Magician"/"MagicBox"),
     * slaveIndex: int,
     * params: Object
    */
    bool setPacket(QJsonObject data);

    QJsonObject getMessage();
    QByteArray getPacketData();
    int getCommondID();
    void setPrintDetailMode(bool en);
    void setPrivate(bool en);
    bool isPrivate();
    void setDeviceType(QString type);
    QString getDeviceType();

private:
    quint64 m_id;
    bool m_isEmpty;
    bool m_isDetailMode;
    PacketPayload m_payload;
    bool m_isPrivate;
    QString m_deviceType;

    quint8 _getCheckSum(QByteArray payload);
};

Q_DECLARE_METATYPE(DPacket)

#endif // DPACKET_H
