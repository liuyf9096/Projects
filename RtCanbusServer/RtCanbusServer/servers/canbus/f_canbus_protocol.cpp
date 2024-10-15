#include "f_canbus_protocol.h"

#include <QByteArray>
#include <QDataStream>
#include <QDebug>

static const quint16 c_head = 0xbfba;
static const quint16 c_end  = 0xeaef;

static quint8 Get_CRC_Value(const QByteArray &data, quint8 length);


/* CAN ID */

FCanidObj::FCanidObj(quint32 canid)
{
    m_canid.value = canid;
}

FCanidObj::FCanidObj(quint16 srcID, quint16 destID, int byteCount)
{
    m_canid.SrcID = srcID;
    m_canid.DestID = destID;

    if (byteCount > 0) {
        int frameCount = byteCount / 8;
        if (byteCount % 8 > 0) {
            frameCount += 1;
        }
        m_canid.count = frameCount;
    }
}

FCanidObj::FCanidObj(const FCanidObj &packet)
{
    m_canid.value = packet.m_canid.value;
}

quint32 FCanidObj::canidFromIndex(quint16 index)
{
    m_canid.index = index;
    return m_canid.value;
}

QString FCanidObj::toString()
{
    return QString("canid:%1 src:0x%2 dest:0x%3 [%4/%5]").arg(m_canid.value, 29, 2, QChar('0'))
            .arg(m_canid.SrcID, 0, 16).arg(m_canid.DestID, 0, 16)
            .arg(m_canid.index).arg(m_canid.count);
}


/* PAYLOAD */

QByteArray FCanBusPayload::encode(quint16 canid, quint16 module, quint16 cmd, const QByteArray &params)
{
    QByteArray result;
    QDataStream in(&result, QIODevice::WriteOnly);
    in.setByteOrder(QDataStream::LittleEndian);

    /* Head (2 byte) */
    in << c_head;

    /* ID (2 byte) */
    in << canid;

    /* module id (2 byte) */
    in << module;

    /* length (2 byte) : cmd(2) + state(2) + params(n) */
    quint16 dataLength = params.count() + 4;
    in << dataLength;

    /* cmd (2 byte) */
    in << cmd;

    /* state (2 byte) */
    in << quint16(0);

    /* params (n byte) */
    if (params.count() > 0) {
        result.append(params);
        in.device()->seek(result.length());
    }

    /* CRC (1 byte) : id, module id, length, cmd, state, params */
    quint8 crc = Get_CRC_Value(result.mid(2), dataLength + 6);
    in << crc;

    /* End (2 byte) */
    in << c_end;

    return result;
}

QJsonObject FCanBusPayload::decode(const QByteArray &rev_data, QByteArray &res_value)
{
    if (rev_data.count() < 15) {
        qDebug() << "[Payload] length error." << res_value.length();
        return QJsonObject();
    }

    QJsonObject obj;
    QByteArray value = rev_data;
    QDataStream out(&value, QIODevice::ReadOnly);
    out.setByteOrder(QDataStream::LittleEndian);

    quint16 head = 0, id = 0, module = 0, length = 0, cmd = 0, state = 0;
    out >> head >> id >> module >> length >> cmd >> state;

    if (head != c_head) {
        qWarning().noquote() << QString("[Payload] head error. 0x%1 != 0x%2").arg(head, 0, 16).arg(c_head, 0, 16);
        return QJsonObject();
    }

    obj.insert("id", id);
    obj.insert("module", module);
    obj.insert("cmd", cmd);
    obj.insert("state", state);

    if (length > 4) {
        res_value = value.mid(12, length - 4);
    }

    out.device()->seek(value.length() - 3);

    quint8 crc = 0;
    out >> crc;
    quint8 crc_c = Get_CRC_Value(value.mid(2, length + 6), length + 6);
    if (crc != crc_c) {
        qWarning() << QString("[Payload] crc error. 0x%1 != 0x%2").arg(crc, 0, 16).arg(crc_c, 0, 16);
        return QJsonObject();
    }

    quint16 end = 0;
    out >> end;
    if (end != c_end) {
        qWarning() << QString("[Payload] end error. 0x%1 != 0x%2").arg(end, 0, 16).arg(c_end, 0, 16);
        return QJsonObject();
    }

    return obj;
}

static quint8 Get_CRC_Value(const QByteArray &data, quint8 length)
{
    quint8 CRC_Value = 0;

    if (data.length() < length) {
        qWarning() << QString("[Payload] crc length error. %1 != %2").arg(data.length()).arg(length);
        return 0;
    }

    for (int i = 0; i < length - 1; i++) {
        quint8 currentByte = data.at(i);
        for (int j = 0; j < 8; j++) {
            if ((CRC_Value >> 7) ^ (currentByte & 0x01)) {
                CRC_Value = (CRC_Value << 1) ^ 0x07;
            } else {
                CRC_Value = (CRC_Value << 1);
            }
            currentByte = currentByte >> 1;
        }
    }
    return CRC_Value;
}
