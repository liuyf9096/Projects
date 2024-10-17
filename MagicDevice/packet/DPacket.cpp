#include "DPacket.h"

#include <QByteArray>
#include <QJsonObject>
#include <QDataStream>
#include <QDebug>

#include "DMagicianProtocol.h"
#include "DM1Protocol.h"

const quint8 Head1 = 0xAA;
const quint8 Head2 = 0xAA;

/****************************************  Payload::Ctrl  ****************************************
 * Bit 0    rw          0-读取 1-写入
 * Bit 1    isQueued    0-非队列指令 1-队列指令
 * Bit 4-5  targetType  00-发送给Magician/MagicianLite
 *                      01-发送给ExController
 * Bit 6-7  targetIndex 0~3-从机序号
 *************************************************************************************************/
PacketCtrl::PacketCtrl()
{
    c_rw = 0;
    c_isQueued = false;
    c_targetType = 0;
    c_slaveid = 0;
}

void PacketCtrl::setPacketCtrl(const quint8 ctrl)
{
    /* [0: rw] */
    c_rw = ctrl & (1 << 0);

    /* [1: isQueued] */
    quint8 isQueue = ctrl & (1 << 1);
    if (isQueue > 0) {
        c_isQueued = true;
    } else {
        c_isQueued = false;
    }

    /* [4~5: type] */
    quint8 type = ctrl & (3 << 4);
    c_targetType = type >> 4;

    /* [6~7: slaveid] */
    quint8 slaveid = ctrl & (3 << 6);
    c_slaveid = slaveid >> 6;
}

void PacketCtrl::clear()
{
    c_rw = 0;
    c_isQueued = false;
    c_targetType = 0;
    c_slaveid = 0;
}

quint8 PacketCtrl::getPacketCtrl()
{
    quint8 res = 0;

    if (c_rw > 0) {
        res |= (1 << 0);
    }
    if (c_isQueued == true) {
        res |= (1 << 1);
    }
    if (c_targetType > 0) {
        res |= (1 << 4);
    }
    if (c_slaveid < 4) {
        res |= (c_slaveid << 6);
    }
    return res;
}

QString PacketCtrl::getTargetType()
{
    return c_targetType == 0 ? QString("Magician/lite") : QString("MagicBox");
}

QString PacketCtrl::getRWType()
{
    return c_rw == 0 ? QString("Read") : QString("Write");
}

int PacketCtrl::getSlaveID()
{
    return c_slaveid;
}

/****************************************  Payload  ****************************************/

PacketPayload::PacketPayload()
{
    p_cmdID = 0;
}

void PacketPayload::setPacketPayload(QByteArray payload)
{
    if (payload.size() < 2) {
        qDebug() << "payload size error.";
        return;
    }

    QDataStream out(&payload, QIODevice::ReadOnly);
    quint8 ctrl = 0;
    out >> p_cmdID >> ctrl;

    p_ctrl.setPacketCtrl(ctrl);

    if (payload.size() >= 2) {
        p_params = payload.right(payload.size() - 2);
    }
}

QByteArray PacketPayload::getPacketPayload()
{
    QByteArray resPayload;
    QDataStream in(&resPayload, QIODevice::WriteOnly);
    in << p_cmdID << p_ctrl.getPacketCtrl();

    if (p_params.size() > 0) {
        resPayload.append(p_params);
    }
    return resPayload;
}

void PacketPayload::clear()
{
    p_cmdID = 0;
    p_ctrl.clear();
    p_params.clear();
}


/****************************************  Packet  ****************************************/
/* Packet:[ head1 | head2 | length | payload(ID | Ctrl | Params(...)) | checksum ] */

DPacket::DPacket(quint64 id) : m_id(id)
{
    m_isDetailMode = false;
    m_isEmpty = true;
    m_isPrivate = false;
}

bool DPacket::isEmpty()
{
    return m_isEmpty;
}

void DPacket::clear()
{
    m_id = 0;
    m_payload.clear();
    m_isEmpty = true;
}

void DPacket::setPacketID(quint64 id)
{
    m_id = id;
}

quint64 DPacket::getPacketID()
{
    return m_id;
}

int DPacket::setPacket(QByteArray data)
{
    m_isEmpty = false;

    /* for detail print */
    if (m_isDetailMode == true) {
        qDebug() << "# packet:" << data.toHex();

        QString parseStr = data.toHex();
        parseStr.insert(4, " l:");
        parseStr.insert(4 + 3 + 2, " (cmdid:");
        parseStr.insert(9 + 8 + 2, " ctrl:");
        parseStr.insert(19 + 6 + 2, " params:");
        parseStr.insert(parseStr.length() - 2, ") checksum:");

        qDebug() << "# content:" << parseStr;
    }

    /* parse content */
    if (data.size() < 5) {
        qDebug() << "packet length error:" << data.size();
        return PARSE_LENGTH_LESS_THAN_5;
    }

    QDataStream out(&data, QIODevice::ReadOnly);
    quint8 h1 = 0, h2 = 0, length = 0;
    out >> h1 >> h2 >> length;

    /* 1.[test HEADER] */
    if (h1 != Head1 || h2 != Head2) {
        qDebug().noquote() << QString("packet header error. 0x%1%2").arg(h1, 0, 16).arg(h2, 0, 16);
        return PARSE_HEAD_ERROR;
    }

    /* 2.[test payload's LENGTH] */
    if (data.size() < 4 + length) {
        qDebug() << "packet length error. target payload length:" << length
                 << "real:" << data.size() - 4;
        return PARSE_LENGTH_NOT_ENOUGH;
    } else if (data.size() > 4 + length) {
        data.resize(4 + length);
        qDebug() << "remove extra data. new data:" << data.toHex();
    }

    /* 3.[test CHECKSUM] */
    QByteArray payload = data.mid(3, length);
    quint8 calCheckSum = _getCheckSum(payload);
    QByteArray checksum_byte = data.right(1).toHex();

    bool ok;
    int checksum = checksum_byte.toInt(&ok, 16);
    if (ok == false || (calCheckSum != checksum)) {
        qDebug() << "checksum is error. get:" << checksum << "cal:" << calCheckSum;
        return PARSE_CHECKSUM_ERROR;
    }

    /* 4.[payload] */
    m_payload.setPacketPayload(payload);

    /* for detail print */
    if (m_isDetailMode == true) {
        qDebug().noquote() << QString("# ctrl (targetType:%1, slaveID:%2, isQueued:%3, rw:%4)")
                              .arg(m_payload.p_ctrl.c_targetType)
                              .arg(m_payload.p_ctrl.c_slaveid)
                              .arg(m_payload.p_ctrl.c_isQueued)
                              .arg(m_payload.p_ctrl.c_rw);
    }
    return PARSE_NO_ERROR;
}

bool DPacket::setPacket(QJsonObject data)
{
    if (!data.contains("cmd")) {
        qDebug() << "packet cmd missing.";
        return false;
    }

    m_isPrivate = data.value("isPrivate").toBool();
    m_isEmpty = false;

    QString cmd = data.value("cmd").toString();
    QJsonObject params = data.value("params").toObject();

    m_payload.p_ctrl.c_isQueued = data.value("isQueued").toBool();

    /* targetType */
    m_deviceType = data.value("targetType").toString();
    if (m_deviceType == "Magician"
            or m_deviceType == "MagicianLite"
            or m_deviceType == "M1") {
        m_payload.p_ctrl.c_targetType = 0;
    } else if (m_deviceType == "MagicBox") {
        m_payload.p_ctrl.c_targetType = 1;
    } else {
        qDebug() << __FUNCTION__ << "targetType is undefined.";
        return false;
    }

    /* slaveID */
    int slaveID = data.value("slaveIndex").toInt(0);
    m_payload.p_ctrl.c_slaveid = static_cast<quint8>(slaveID);

    bool ok = false;
    if (m_deviceType == "M1") {
        ok = DM1Protocol::getInstance()->setPayload(cmd, m_payload, params);
    } else {
        ok = DMagicianProtocol::getInstance()->setPayload(cmd, m_payload, params);
    }
    return ok;
}

/* 求校验和 */
quint8 DPacket::_getCheckSum(QByteArray payload)
{
    quint8 sum = 0;
    for (int i = 0; i < payload.size(); ++i) {
        int t = payload.at(i);
        sum += static_cast<quint8>(t);
    }
    quint8 res = static_cast<quint8>(256 - sum);
    return res;
}

QJsonObject DPacket::getMessage()
{
    QJsonObject resObj;
    resObj.insert("id", static_cast<double>(m_id));
    resObj.insert("cmdid", m_payload.p_cmdID);
    resObj.insert("rw", m_payload.p_ctrl.c_rw);
    resObj.insert("rwType", m_payload.p_ctrl.getRWType());
    resObj.insert("isQueued", m_payload.p_ctrl.c_isQueued);
    resObj.insert("targetType", m_payload.p_ctrl.getTargetType());
    resObj.insert("slaveID", m_payload.p_ctrl.c_slaveid);

    if (m_isPrivate == true) {
        resObj.insert("isPrivate", true);
    }

    QJsonObject parseObj;
    if (m_deviceType == "M1") {
        parseObj = DM1Protocol::getInstance()->parseParams(m_payload);
    } else {
        parseObj = DMagicianProtocol::getInstance()->parseParams(m_payload);
    }

    if (parseObj.contains("cmd")) {
        resObj.insert("cmd", parseObj.value("cmd").toString());
    }
    if (parseObj.contains("params")) {
        resObj.insert("params", parseObj.value("params").toObject());
    }
    return resObj;
}

QByteArray DPacket::getPacketData()
{
    QByteArray resData;
    QDataStream in(&resData, QIODevice::WriteOnly);
    QByteArray payload = m_payload.getPacketPayload();
    in << Head1 << Head2 << quint8(payload.size());

    resData.append(payload);

    in.device()->seek(resData.size());
    in << _getCheckSum(payload);
    return resData;
}

int DPacket::getCommondID()
{
    return m_payload.p_cmdID;
}

void DPacket::setPrintDetailMode(bool en)
{
    m_isDetailMode = en;
}

void DPacket::setPrivate(bool en)
{
    m_isPrivate = en;
}

bool DPacket::isPrivate()
{
    return m_isPrivate;
}

void DPacket::setDeviceType(QString type)
{
    m_deviceType = type;
}

QString DPacket::getDeviceType()
{
    return m_deviceType;
}




