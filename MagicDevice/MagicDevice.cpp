#include "MagicDevice.h"
#include "MagicDevice_p.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include "DPacket.h"
#include "MessageHandler.h"
#include "DMagicianProtocol.h"
#include "DM1Protocol.h"
#include "DError.h"

const QString VERSION = QString("3.4.2");

MessagePacket::MessagePacket(QString cmd, quint64 id) : id(id), m_cmd(cmd)
{
    m_isQueued = false;
    m_slaveID = 0;
    m_cmdid = 0;
    m_rw = 0;
    m_targetType = DEVICE_MAGICIAN;
    m_isPrivate = false;
    m_isWaitForFinish = false;
}

bool MessagePacket::isEmpty()
{
    return m_cmd.isEmpty();
}

void MessagePacket::setCommand(QString cmd)
{
    m_cmd = cmd;
}

void MessagePacket::setIsQueue(bool isQueued)
{
    m_isQueued = isQueued;
}

bool MessagePacket::getIsQueue()
{
    return m_isQueued;
}

void MessagePacket::setParams(QJsonObject params)
{
    if (!params.isEmpty()) {
        m_params = params;
    }
}

void MessagePacket::setSlaveID(quint8 slaveid)
{
    if (slaveid < 4) {
        m_slaveID = slaveid;
    }
}

quint8 MessagePacket::getSlaveID()
{
    return m_slaveID;
}

void MessagePacket::setTargetType(DeviceType type)
{
    m_targetType = type;
}

void MessagePacket::setTargetType(QString typeStr, quint8 slaveIndex)
{
    int type = MagicDevicePrivate::DeviceTypeMap.key(typeStr);
    m_targetType = DeviceType(type);
    setSlaveID(slaveIndex);
}

void MessagePacket::setWaitForFinishEnable(bool enable, int timeout)
{
    m_isWaitForFinish = enable;
    m_timeout = timeout;
}

bool MessagePacket::getIsWaitForFinish()
{
    return m_isWaitForFinish;
}

int MessagePacket::getTimeoutValue()
{
    return m_timeout;
}

QJsonObject MessagePacket::getParamsObj()
{
    return m_params;
}

QString MessagePacket::getCmdStr()
{
    return m_cmd;
}

void MessagePacket::setPrivatePacket(bool isPrivate)
{
    m_isPrivate = isPrivate;
}

DeviceType MessagePacket::getTargetType()
{
    return m_targetType;
}

QString MessagePacket::getTargetTypeStr()
{
    return MagicDevicePrivate::DeviceTypeMap.value(m_targetType);
}

QJsonObject MessagePacket::getPacketObj()
{
    QJsonObject resObj;
    resObj.insert("id", static_cast<double>(id));
    resObj.insert("cmd", m_cmd);
    resObj.insert("isQueued", m_isQueued);
    resObj.insert("targetType", MagicDevicePrivate::DeviceTypeMap.value(m_targetType));
    resObj.insert("slaveIndex", m_slaveID);

    if (!m_params.isEmpty()) {
        resObj.insert("params", m_params);
    }
    if (m_isPrivate) {
        resObj.insert("isPrivate", true);
    }
    /* targetType 为信息包发送的目的单元 */
    /* deviceType 则表示信息的来源设备 */
    return resObj;
}

/* MagicDevicePrivate */

QMap<int, QString> MagicDevicePrivate::DeviceTypeMap = initDeviceTypeStringMap();
QMap<int, QString> MagicDevicePrivate::initDeviceTypeStringMap()
{
    QMap<int, QString> map;
    map.insert(DEVICE_MAGICIAN, "Magician");
    map.insert(DEVICE_MAGICIAN_LITE, "MagicianLite");
    map.insert(DEVICE_MAGICBOX, "MagicBox");
    map.insert(DEVICE_M1, "M1");
    return map;
}

QString MagicDevicePrivate::checkIsIpAddress(QString ip)
{
    QRegExp rx("((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)");
    if (rx.indexIn(ip) > -1) {
        return rx.cap();
    }
    return QString();
}

MagicDevicePrivate::MagicDevicePrivate(MagicDevice *parent) : q_ptr(parent)
{
    poseX = 0;
    poseY = 0;
    poseZ = 0;
    poseR = 0;

    m_wsport = 0;
    m_deviceType = 0;

    m_status = UNCONNECTED;

    m_MessageHandler = new MessageHandler();
    connect(m_MessageHandler, &MessageHandler::onConnectStatus_signal,
            this, &MagicDevicePrivate::handleConnectResult_slot);
    connect(m_MessageHandler, &MessageHandler::sendMessages_signal,
            this, &MagicDevicePrivate::handleCommonResult_slot);
    connect(m_MessageHandler, &MessageHandler::onErrorOccurred_signal,
            this, &MagicDevicePrivate::handleErrorResult_slot);

    m_thread = new QThread();
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_MessageHandler->moveToThread(m_thread);
    m_thread->start();

    m_actionTimerManager = new ActionTimerManager(this);
    connect(m_actionTimerManager, &ActionTimerManager::getCurrentIndex_signal,
            this, &MagicDevicePrivate::checkCurrentIndex_slot);
    connect(m_actionTimerManager, &ActionTimerManager::actionFinish_signal,
            this, &MagicDevicePrivate::handleActionTimeout_slot);
}

MagicDevicePrivate::~MagicDevicePrivate()
{
    m_thread->quit();
    bool ok = m_thread->wait();
    qDebug() << "Thread[MagicDevice] quit:" << ok;

    m_MessageHandler->deleteLater();
    qDebug() << __FUNCTION__ << "finish";
}

/* 发送获取queued index指令 */
void MagicDevicePrivate::checkCurrentIndex_slot()
{
    Q_Q(MagicDevice);

    MessagePacket packet("GetQueuedCmdCurrentIndex");
    packet.setTargetType(m_actionTimerManager->getCurrentTargetType(),
                         m_actionTimerManager->getCurrentSlaveIndex());
    packet.setPrivatePacket(true);
    q->sendCommand(packet);
}

/* 收到[连接结束] */
void MagicDevicePrivate::handleConnectResult_slot(int code, quint64 id)
{
#ifndef QT_NO_DEBUG
    qDebug() << __FUNCTION__ << QString("id:%1, error code:%2").arg(id).arg(code);
#endif

    if (code == QSerialPort::NoError) {
        m_status = CONNECTED;
    } else if (code == QSerialPort::OpenError) {
        m_status = OCCUPIED;
    } else {
        m_status = UNCONNECTED;
    }

    Q_Q(MagicDevice);
    emit q->onResultMessage_signal(id, "ConnectDobot", code);

    m_RequestMap.remove(id);
}

/* 收到[错误]:断开连接 通讯超时 队列满 */
void MagicDevicePrivate::handleErrorResult_slot(int code, quint64 id)
{
#ifndef QT_NO_DEBUG
    qDebug() << __FUNCTION__ << QString("id:%1, error code:%2").arg(id).arg(code);
#endif

    m_PacketListMap.remove(id);
    MessagePacket packet = m_RequestMap.take(id);

    Q_Q(MagicDevice);
    emit q->onResultMessage_signal(id, packet.getCmdStr(), code);
}

/* 收到[common 结果] */
void MagicDevicePrivate::handleCommonResult_slot(QJsonObject message)
{
    Q_Q(MagicDevice);

    quint64 id = static_cast<quint64>(message.value("id").toDouble());
    QString cmd = message.value("cmd").toString();

#ifndef QT_NO_DEBUG
    qDebug().noquote() << __FUNCTION__ << QString("id:%1, cmd:%2").arg(id).arg(cmd) << message;
#endif

    if (cmd == "GetPose") {
        QJsonObject poseParams = message.value("params").toObject();

        poseX = static_cast<float>(poseParams.value("x").toDouble());
        poseY = static_cast<float>(poseParams.value("y").toDouble());
        poseZ = static_cast<float>(poseParams.value("z").toDouble());
        poseR = static_cast<float>(poseParams.value("r").toDouble());
    }

    if (message.value("isPrivate").toBool() == true) {
        /* 检测是否有自发请求 */
        if (cmd == "GetQueuedCmdCurrentIndex") {
            QJsonObject paramsObj = message.value("params").toObject();
            if (paramsObj.contains("queuedCmdIndex")) {
                quint64 currentIndex = static_cast<quint64>(paramsObj.value("queuedCmdIndex").toDouble());
                m_actionTimerManager->updateCurrentIndex(currentIndex);
#if 0
                quint64 id = m_actionTimerManager->getCurrentHandlingId();
                MessagePacket packet = m_RequestMap.take(id);
                emit q->onResultMessage_signal(id, packet.getCmdStr(), NoError);
#endif
            }
        }
    } else if (m_actionTimerManager->containsTimerId(id)) {
        /* 检测是否有需要动作等待结束的指令 */
        /* 当发送队列指令后，会返回带有index的参数，获取该参数用以对比 */
        if (message.contains("params")) {
            QJsonObject paramsObj = message.value("params").toObject();
            quint64 targetIndex = static_cast<quint64>(paramsObj.value("queuedCmdIndex").toDouble());
            m_actionTimerManager->setTargetIndexWithId(id, targetIndex);
        } else {
            qDebug() << "no params (queuedCmdIndex) return.";
            m_RequestMap.remove(id);
        }
    } else if (m_PacketListMap.contains(id)) {
        /* 是否有列队顺序执行指令 */
        m_RequestMap.remove(id);

        QList<MessagePacket> list = m_PacketListMap.value(id);
        qDebug() << "request cmd list remain:" << list.count();

        if (!list.isEmpty()) {
            MessagePacket p = list.takeFirst();
            bool ok = q->sendCommand(p);
            if (ok && !list.isEmpty()) {
                m_PacketListMap.insert(id, list);
            } else {
                m_PacketListMap.remove(id);
            }
        } else {
            m_PacketListMap.remove(id);
        }
    } else {
        /* 常规结束指令 */
        emit q->onResultMessage_signal(id, cmd, NoError, message.value("params"));
        m_RequestMap.remove(id);
    }
}

/* 运动结束 / 运动超时 */
void MagicDevicePrivate::handleActionTimeout_slot(quint64 id, ActionTimerManager::FinishType type)
{
    if (m_RequestMap.contains(id)) {
        MessagePacket packet = m_RequestMap.take(id);

        Q_Q(MagicDevice);

        if (type == ActionTimerManager::NOERROR)
        {
            emit q->onResultMessage_signal(id, packet.getCmdStr(), NoError);
            if (m_PacketListMap.contains(id)) {
                QList<MessagePacket> list = m_PacketListMap.value(id);
                qDebug() << "request cmd list remain:" << list.count();

                if (!list.isEmpty()) {
                    MessagePacket p = list.takeFirst();
                    bool ok = q->sendCommand(p);
                    if (ok && !list.isEmpty()) {
                        m_PacketListMap.insert(id, list);
                    } else {
                        m_PacketListMap.remove(id);
                    }
                } else {
                    m_PacketListMap.remove(id);
                }
            }
        }
        else if (type == ActionTimerManager::TIMEOUT)
        {
            emit q->onResultMessage_signal(id, packet.getCmdStr(), DeviceActionTimeoutError);
            if (m_PacketListMap.contains(id)) {
                m_PacketListMap.remove(id);
            }
            m_RequestMap.remove(id);
        }
        else if (type == ActionTimerManager::CANCELED)
        {
            emit q->onResultMessage_signal(id, packet.getCmdStr(), DeviceActionCanceled);
            if (m_PacketListMap.contains(id)) {
                m_PacketListMap.remove(id);
            }
            m_RequestMap.remove(id);
        }
    } else {
        qDebug() << "can not find request id.";
    }
}


/* MagicDevice */

MagicDevice::MagicDevice(QObject *parent)
    : QObject(parent), Dptr(new MagicDevicePrivate(this))
{
    Q_D(MagicDevice);
    d->m_deviceType = DEVICE_UNKNOWN;
}

MagicDevice::MagicDevice(DeviceType type, QObject *parent)
    : QObject(parent), Dptr(new MagicDevicePrivate(this))
{
    Q_D(MagicDevice);
    d->m_deviceType = type;
}

MagicDevice::~MagicDevice()
{
    Q_D(MagicDevice);
    delete d;
}

QString MagicDevice::getDeviceTypeToString(DeviceType type)
{
    return MagicDevicePrivate::DeviceTypeMap.value(type);
}

DeviceType MagicDevice::getDeviceStringToType(QString type)
{
    int t = MagicDevicePrivate::DeviceTypeMap.key(type);
    return DeviceType(t);
}

QString MagicDevice::getProtocolVersion()
{
    return VERSION;
}

void MagicDevice::setDeviceType(DeviceType type)
{
    Q_D(MagicDevice);
    d->m_deviceType = type;

    if (type == DEVICE_M1) {
        d->m_MessageHandler->setProtocolType(MessageHandler::DEVICE_M1_PROTOCOL);
    } else {
        d->m_MessageHandler->setProtocolType(MessageHandler::DEVICE_MAGIC_PROTOCOL);
    }
}

DeviceType MagicDevice::getDeviceType()
{
    Q_D(MagicDevice);
    return DeviceType(d->m_deviceType);
}

void MagicDevice::setPortName(QString portName)
{
    Q_D(MagicDevice);

#if 0
    QRegExp rx("((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)");
    if (rx.indexIn(portName) > -1) {
        d->m_MessageHandler->setUdpIpAddress(rx.cap());
    } else {
        d->m_MessageHandler->setPortName(portName);
    }
#else
    QString ip = MagicDevicePrivate::checkIsIpAddress(portName);
    if (!ip.isEmpty()) {
        d->m_MessageHandler->setUdpIpAddress(ip);
    } else {
        d->m_MessageHandler->setPortName(portName);
    }
#endif
}

QString MagicDevice::getPortName()
{
    Q_D(MagicDevice);
    return d->m_MessageHandler->getPortName();
}

void MagicDevice::setWebsocketPort(quint16 wsport)
{
    Q_D(MagicDevice);
    d->m_wsport = wsport;
}

quint16 MagicDevice::getWebsocketPort()
{
    Q_D(MagicDevice);
    return d->m_wsport;
}

void MagicDevice::setHostIpAddress(QString ip)
{
    Q_D(MagicDevice);
    d->m_MessageHandler->setHostIpAddress(ip);
}

void MagicDevice::setDeviceIpAddress(QString ip)
{
    Q_D(MagicDevice);
    d->m_MessageHandler->setUdpIpAddress(ip);
}

/* Connect Device */
void MagicDevice::connectDevice(quint64 requestid)
{
    Q_D(MagicDevice);
    if (!d->m_MessageHandler->getPortName().isEmpty()) {
        QMetaObject::invokeMethod(d->m_MessageHandler,
                                  "connectDevice",
                                  Qt::AutoConnection,
                                  Q_ARG(quint64, requestid));
    } else {
        qDebug() << "portName is not specified.";
    }
}

/* DisConnect Device */
void MagicDevice::disConnectDevice(quint64 requestid)
{
    Q_D(MagicDevice);
    d->m_status = UNCONNECTED;

    d->m_actionTimerManager->clearTimers();
    d->m_PacketListMap.clear();

    QMetaObject::invokeMethod(d->m_MessageHandler,
                              "disconnectDevice",
                              Qt::AutoConnection,
                              Q_ARG(quint64, requestid));

    d->m_RequestMap.clear();
}

QString MagicDevice::getConnectStatus()
{
    Q_D(MagicDevice);

    if (d->m_status == UNCONNECTED) {
        return QString("unconnected");
    } else if (d->m_status == CONNECTED) {
        return QString("connected");
    } else if (d->m_status == OCCUPIED) {
        return QString("occupied");
    }
    return QString("unknown");
}

/* Set time out value */
void MagicDevice::setCommuTimeout(int ms)
{
    if (ms > 0) {
        Q_D(MagicDevice);
        QMetaObject::invokeMethod(d->m_MessageHandler,
                                  "setTimeoutValue",
                                  Qt::AutoConnection,
                                  Q_ARG(int, ms));
    }
}

bool MagicDevice::sendCommand(MessagePacket &packet, bool connectCheck)
{
    Q_D(MagicDevice);
    if (connectCheck && d->m_MessageHandler->isConnected() == false) {
        qDebug() << "device is NOT connected.";
        return false;
    }

    if (packet.getIsQueue() == true and packet.getIsWaitForFinish() == true) {
        if (!d->m_actionTimerManager->containsTimerId(packet.id)) {
            ActionTimerSet *timer = new ActionTimerSet(packet.id);
            timer->cmd = packet.getCmdStr();
            timer->timeoutValue = packet.getTimeoutValue();
            timer->targetType = packet.getTargetTypeStr();

            d->m_actionTimerManager->addTimer(timer);
        } else {
            qDebug() << "packet id error. No timer is set.";
        }
    }

    if (packet.getCmdStr() == "SetRCmd") {
        packet.setCommand("SetPTPCmd");

        QJsonObject paramsObj = packet.getParamsObj();
        paramsObj.insert("ptpMode", 1);

        paramsObj.insert("x", static_cast<double>(d->poseX));
        paramsObj.insert("y", static_cast<double>(d->poseY));
        paramsObj.insert("z", static_cast<double>(d->poseZ));
        packet.setParams(paramsObj);
    }

#ifndef QT_NO_DEBUG
    qDebug() << "insert cmd:" << packet.id << packet.getCmdStr();
#endif

    d->m_RequestMap.insert(packet.id, packet);

    DPacket d_packet(packet.id);
    QJsonObject dataObj = packet.getPacketObj();
    if (d_packet.setPacket(dataObj) == true) {
        QMetaObject::invokeMethod(d->m_MessageHandler,
                                  "addSendingList",
                                  Qt::QueuedConnection,
                                  Q_ARG(DPacket, d_packet));
    } else {
        return false;
    }
    return true;
}

bool MagicDevice::sendCommandList(quint64 id, DPacketList packetList, bool enCheck)
{
    if (packetList.isEmpty()) {
        qDebug() << "packet list is empty";
        return false;
    }

    MessagePacket packet = packetList.takeFirst();
    Q_D(MagicDevice);
    d->m_PacketListMap.insert(id, packetList);

    bool ok = sendCommand(packet, enCheck);
    return ok;
}

void MagicDevice::stopQueueCmdTimer()
{
    Q_D(MagicDevice);
    d->m_actionTimerManager->clearTimers();
}

/* 公共函数 */
QStringList MagicDevice::getCommandList(QString deviceType)
{
    if (deviceType.compare("M1", Qt::CaseInsensitive) == 0) {
        return DM1Protocol::getInstance()->getCommandList();
    } else {
        return DMagicianProtocol::getInstance()->getCommandList();
    }
}

/* 暂时不用 */
QStringList MagicDevice::getDeviceList(bool isFiltered)
{
    QStringList resList;

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &portInfo, availablePorts) {
        if (isFiltered == true) {
            if (portInfo.description().contains("USB")
                    || portInfo.description().contains("CH340")
                    || portInfo.description().contains("CP210")
                    || portInfo.description().contains("Magician")) {
                resList.append(portInfo.portName());
            }
        } else {
            resList.append(portInfo.portName());
        }
    }
    return resList;
}
