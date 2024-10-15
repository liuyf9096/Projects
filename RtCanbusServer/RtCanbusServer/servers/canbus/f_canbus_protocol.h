#ifndef F_CANBUS_PROTOCOL_H
#define F_CANBUS_PROTOCOL_H

#include <QtCore>
#include <QByteArray>
#include <QJsonObject>

/*******************************************************
 *  CANID (29 bit):
 *  [28:26]     [25:18]     [17:10]     [9:5]   [4:0]
 *  reserve     src_id      dest_id     count	index
 *  3bit        8bit        8bit		5bit	5bit
 *******************************************************/
typedef union CanID_U {
    quint32 value = 0;
    struct {
        unsigned index : 5;     // frame index
        unsigned count : 5;     // total frame count
        unsigned SrcID : 8;     // sender ID
        unsigned DestID : 8;    // receiver ID
        unsigned reserved : 3;  // reserved

        unsigned ErrFlag : 1;
        unsigned RtrFlag : 1;
        unsigned EffFlag : 1;
    };
} CanID_U;


/* CAN ID */
class FCanidObj
{
public:
    explicit FCanidObj(quint32 canid);
    FCanidObj(quint16 srcID, quint16 destID, int byteCount);
    FCanidObj(const FCanidObj &packet);

    void setCount(quint16 count) { m_canid.count = count; }
    int count() { return m_canid.count; }

    void setIndex(quint16 index) { m_canid.index = index; }
    int index() { return m_canid.index; }

    void setSrcId(quint16 scrid) { m_canid.SrcID = scrid; }
    int srcid() { return m_canid.SrcID; }

    void setDestId(quint16 destid) { m_canid.DestID = destid; }
    int destid() { return m_canid.DestID; }

    quint32 canidFromIndex(quint16 index);

    QString toString();

private:
    CanID_U m_canid;
};


/***************************************************************************************************
 *  PAYLOAD:
 *  head        can_id      module      length      cmd     state   params  crc             end
 *  [2Byte]     [2Byte]     [2Byte]     [2Byte]     [2Byte] [2Byte] [nByte] [1Byte]         [2Byte]
 *  0xbfba      0~65535     0           4+params	''      1~6     ''      f(id..params)   0xeaef
 ***************************************************************************************************/

class FCanBusPayload
{
public:
    static QByteArray encode(quint16 canid, quint16 module, quint16 cmd, const QByteArray &params = QByteArray());
    static QJsonObject decode(const QByteArray &rev_data, QByteArray &res_value);
};

/* decode return:
 * id       (int)
 * module   (int)
 * cmd      (int)
 * state    (int)
 * res_value(array)
*/

#endif // F_CANBUS_PROTOCOL_H
