#include "MagicDevicePlugin.h"

#include <QList>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QDebug>

#include "DProgramDownload.h"

#define DEVICE_DLL_ERROR_BASE (120)

const QString MagicDevicePlugin::PluginName = "MagicDevice";
const QString MagicDevicePlugin::Version = "3.2.3";
const QByteArray BroadCastMessage = "WhoisDobotM1";
const quint16 BroadCastPort = 6000;

MagicDevicePlugin::MagicDevicePlugin(QObject *parent) : DPluginInterface(parent)
{
    m_handlingid = 0;
    m_researchFileter = false;
    m_isSearchingDevices = false;

    downloader = new DProgramDownload(this);
    connect(downloader, &DProgramDownload::onDownloadFinished_signal,
            this, &MagicDevicePlugin::handleDownloadFinished_slot);

    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &MagicDevicePlugin::onUdpReadyRead_slot);

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(100);
    connect(m_searchTimer, &QTimer::timeout,
            this, &MagicDevicePlugin::onSearchTimeout_slot);
}

QString MagicDevicePlugin::getVersion()
{
    QString protocolV = MagicDevice::getProtocolVersion();
    QString version = QString("%1(protocal:%2)").arg(Version).arg(protocolV);
    return version;
}

/* 收到消息 */
void MagicDevicePlugin::pReceiveMassage_slot(QString id, QJsonObject obj)
{
    if (id == "ALL") {
        handleDobotLinkCommand(obj);
    } else if (id.contains(PluginName)) {
        handleMagicDeviceCommand(obj);
    }
}

/* [!!!消息分发!!!] */
void MagicDevicePlugin::handleDobotLinkCommand(const QJsonObject &obj)
{
    qDebug() << "[ALL] {MagicDevice} get obj" << obj;
    if (obj.contains("METHOD")) {
        QString method = obj.value("METHOD").toString();

        if (method == "EXIT") {
            _closeAllDevice();
        } else if (method == "CloseWebSocket") {
            QJsonObject params = obj.value("params").toObject();
            quint16 port = static_cast<quint16>(params.value("WSport").toInt());
            pDisconnectDobot(port);
        }
    }
}

void MagicDevicePlugin::handleMagicDeviceCommand(const QJsonObject &obj)
{
    DRequestPacket packet;
    packet.setPacketFromObj(obj);
    DResultPacket resPacket(packet);

    if (m_requestPacketMap.contains(packet.id)) {
        qWarning() << tr("The same id can cause system instability.");
    }

    m_requestPacketMap.insert(packet.id, packet);

    if (packet.api == "SearchDobot") {
        pSearchDobot(packet);
    } else {
        /* check portName */
        if (packet.getParamValue("portName").toString().isEmpty()) {
            m_requestPacketMap.remove(packet.id);

            emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_INVALID_PORTNAME));
        }
        else
        {
            if (packet.api == "ConnectDobot") {
                pConnectDobot(packet);
            }
            else
            {
                MagicDevice *device = _getDevice(packet);
                if (device)
                {
                    if (packet.api == "DisconnectDobot") {
                        pDisconnectDobot(device, packet);
                    } else if (packet.api == "DownloadProgram") {
                        handleDownloadCmd(device, packet);
                    } else if (packet.api == "QueuedCmdStop") {
                        pQueuedCmdStop(device, packet);
                    }
                    else
                    {
                        bool ok = false;
                        if (_checkActionApi(packet.api)) {
                            ok = handleActionCmd(device, packet);
                        } else if (_checkQueueApi(packet.api)) {
                            ok = handleQueueCmd(device, packet);
                        }  else {
                            ok = pSendCommand(device, packet);
                        }
                        if (ok == false) {
                            emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_INVALID_COMMAND));
                        }
                    }
                } else {
                    emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_INVALID_DEVICE));
                }
            }
        }
    }
}

MagicDevice *MagicDevicePlugin::_getDevice(const DRequestPacket &packet)
{
    QString portName = packet.getParamValue("portName").toString();
    if (m_deviceMap.contains(portName)) {
        MagicDevice *device = m_deviceMap.value(portName);
        return device;
    } else {
        m_requestPacketMap.remove(packet.id);

        DResultPacket resPacket(packet);
        emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_INVALID_DEVICE));
        return nullptr;
    }
}

void MagicDevicePlugin::_getAvailableSerialPort(QStringList filter)
{
    QList<QSerialPortInfo> list;
    foreach (const QSerialPortInfo &portInfo, QSerialPortInfo::availablePorts()) {

        DeviceInfo info;
        info.portName = portInfo.portName();
        info.description = portInfo.description();
        info.status = "unconnected";

        if (m_deviceMap.contains(portInfo.portName())) {
            MagicDevice *device = m_deviceMap.value(portInfo.portName());
            if (device) {
                /* unconnected/connected/occupied/unknown */
                info.status = device->getConnectStatus();
            }
        }

        m_preDeviceMap.insert(portInfo.portName(), info);
    }

    if (!filter.isEmpty()) {
        foreach (const QString portName, m_preDeviceMap.keys()) {
            foreach (const QString &f, filter) {
                if (portName.contains(f, Qt::CaseInsensitive)) {
                    m_preDeviceMap.remove(portName);
                    break;
                }
            }
        }
    }
}

void MagicDevicePlugin::pSearchDobot(const DRequestPacket &packet)
{
    qDebug() << __FUNCTION__;

    if (m_isSearchingDevices == true) {
        _sendErrorMessage(packet, ERROR_API_BUSY);
        return;
    }

    m_isSearchingDevices = true;

    QString filter = packet.getParamValue("filter").toString();
    QStringList filterList = filter.split(" ", QString::SkipEmptyParts);
    bool checkConnection = packet.getParamValue("connectCheck").toBool(false);
    QString type = packet.target;
    bool isM1 = (type.compare("M1", Qt::CaseInsensitive) == 0) ? true : false;

    m_preDeviceMap.clear();

    _getAvailableSerialPort(filterList);

    if (checkConnection == false and isM1 == false) {
        QJsonArray array = _getSearchResult();
        _sendResMessage(packet, array);
        m_isSearchingDevices = false;
    } else {
        if (checkConnection == true) {
            m_handlingSearchDeviceList.clear();

            DeviceType t = MagicDevice::getDeviceStringToType(type);
            foreach (const QString &portName, m_preDeviceMap.keys()) {
                _checkDeviceWithType(portName, t, packet.id);
            }
        }
        if (isM1 == true) {
            m_m1deviceMap.clear();
            m_searchTimer->setProperty("id", packet.id);
            _broadcastForSearchM1();
        }
    }
}

void MagicDevicePlugin::_checkDeviceWithType(QString portName, DeviceType type, quint64 id)
{
    qDebug() << "check portName:" << portName << "type:" << type << "id:" << id;

    m_handlingSearchDeviceList.append(portName);

    MagicDevice *device = new MagicDevice(type, this);
    connect(device, &MagicDevice::onResultMessage_signal,
            this, &MagicDevicePlugin::handleCheckDeviceType_slot);

    device->setPortName(portName);
    device->connectDevice(id);

    MessagePacket p("GetProductName", id);
    p.setTargetType(type);
    device->sendCommand(p, false);
}

QJsonArray MagicDevicePlugin::_getSearchResult()
{
    QJsonArray devicesArray;
    foreach (const DeviceInfo &info, m_preDeviceMap.values()) {
        QJsonObject deviceObj;
        deviceObj.insert("portName", info.portName);
        deviceObj.insert("description", info.description);
        deviceObj.insert("status", info.status);
        devicesArray.append(deviceObj);
    }
    return devicesArray;
}

void MagicDevicePlugin::pConnectDobot(const DRequestPacket &packet)
{
    QString portName = packet.getParamValue("portName").toString();
    qDebug() << __FUNCTION__ << QString("id:%1, portName:%2").arg(packet.id).arg(portName);

    MagicDevice *device = m_deviceMap.value(portName);
    if (device == nullptr) {
        device = new MagicDevice(this);
        device->setPortName(portName);
        connect(device, &MagicDevice::onResultMessage_signal,
                this, &MagicDevicePlugin::handleReceiveMessage_slot);
        qDebug() << "create a device. portName:" << portName << "wsPort:" << packet.wsPort;
        m_deviceMap.insert(portName, device);
    }

    /* 绑定 websocket 端口 */
    device->setWebsocketPort(packet.wsPort);

    if (packet.target.compare("Magician", Qt::CaseInsensitive) == 0) {
        device->setDeviceType(DEVICE_MAGICIAN);
    } else if (packet.target.compare("MagicianLite", Qt::CaseInsensitive) == 0) {
        device->setDeviceType(DEVICE_MAGICIAN_LITE);
    } else if (packet.target.compare("MagicBox", Qt::CaseInsensitive) == 0) {
        device->setDeviceType(DEVICE_MAGICBOX);
    } else if (packet.target.compare("M1", Qt::CaseInsensitive) == 0) {
        /* M1 */
        qDebug() << "set localIP:" << m_localIp;
        device->setHostIpAddress(m_localIp);
        device->setDeviceType(DEVICE_M1);
    } else {
        qDebug() << "Device type is not specified.";
    }

    device->connectDevice(packet.id);

    if (packet.paramsObj.value("queueStart").toBool(true)) {
        MessagePacket p("QueuedCmdStart");
        p.setTargetType(packet.target);
        device->sendCommand(p, false);
    }
}

void MagicDevicePlugin::pDisconnectDobot(MagicDevice *device, const DRequestPacket &packet)
{
    QString portName = packet.getParamValue("portName").toString();
    qDebug() << __FUNCTION__ << QString("id:%1, portName:%2").arg(packet.id).arg(portName);

    if (device) {
        device->disConnectDevice(packet.id);
    }
}

void MagicDevicePlugin::pDisconnectDobot(quint16 wsport)
{
    qDebug() << __FUNCTION__ << "websocket port:" << wsport;

    foreach (MagicDevice *device, m_deviceMap.values()) {
        if (device && device->getWebsocketPort() == wsport) {
            device->disConnectDevice();
        }
    }
}

/* 检查是否为运动等待指令 */
bool MagicDevicePlugin::_checkActionApi(QString api)
{
    QStringList actionList;
    actionList << "SetHOMECmd" << "SetPTPCmd" << "SetPTPWithLCmd"
               << "SetPTPPOCmd" << "SetPTPPOWithLCmd"
               << "SetCPCmd" << "SetCPLECmd" << "SetARCCmd" << "SetCircleCmd"
               << "SetMotivateCmd" << "SetTRIGCmd";

    return actionList.contains(api);
}

/* 设置默认等待运动时间 */
int MagicDevicePlugin::_getDefaultTimeoutValue(QString devideType, QString cmd)
{
    int defaultTimeout = 5000;
    if (cmd == "SetHOMECmd") {
        if (devideType == "Magician") {
            defaultTimeout = 30000;
        } else if (devideType == "MagicianLite") {
            defaultTimeout = 8000;
        } else if (devideType == "MagicBox") {
            defaultTimeout = 30000;
        } else if (devideType == "M1") {
            defaultTimeout = 60000;
        } else {
            defaultTimeout = 5000;
        }
    } else {
        if (devideType == "M1") {
            defaultTimeout = 60000;
        } else {
            defaultTimeout = 5000;
        }
    }
    return defaultTimeout;
}

/* 需要运动等待的 api */
bool MagicDevicePlugin::handleActionCmd(MagicDevice *device, const DRequestPacket &packet)
{
    qDebug() << __FUNCTION__ << QString("id:%1, api:%2").arg(packet.id).arg(packet.api);

    MessagePacket sendPacket(packet.api, packet.id);
    sendPacket.setParams(packet.paramsObj);

    if (!packet.paramsObj.contains("isQueued")) {
        qDebug() << "'isQueued':missing, use (true)";
    }
    if (!packet.paramsObj.contains("isWaitForFinish")) {
        qDebug() << "'isWaitForFinish':missing, use (true)";
    }

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);
    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    sendPacket.setIsQueue(isQueued);

    if (isQueued == true and isWaitForFinish == true) {
        int defaultValue = _getDefaultTimeoutValue(packet.target, packet.api);
        if (!packet.paramsObj.contains("timeout")) {
            qDebug().noquote() << QString("'timeout':missing, use(%1)").arg(defaultValue);
        }

        int timeout = packet.paramsObj.value("timeout").toInt(defaultValue);
        sendPacket.setWaitForFinishEnable(isWaitForFinish, timeout);
    }

    if (packet.target == "MagicianLite" and !packet.paramsObj.contains("slaveIndex")) {
        qDebug() << "'slaveIndex':missing, use (0)";
    }

    quint8 slaveIndex = static_cast<quint8>(packet.paramsObj.value("slaveIndex").toInt());
    sendPacket.setTargetType(packet.target, slaveIndex);

    if (device) {
        bool ok = device->sendCommand(sendPacket);
        return ok;
    }
    return false;
}

/* 检查是否为队列（非运动等待）指令 */
bool MagicDevicePlugin::_checkQueueApi(QString api)
{
    if (!api.startsWith("Set")) {
        return false;
    }

    if (api.endsWith("Params") || api.endsWith("Cmd") || api.endsWith("Sensor")) {
        return true;
    }

    if (api.contains("EndEffector")) {
        return true;
    }

    if (api.startsWith("SetIO")) {
        return true;
    }

    if (api == "SetEMotor" || api == "SetAutoLeveling") {
        return true;
    }
    return false;
}

/* 处理队列指令 API */
bool MagicDevicePlugin::handleQueueCmd(MagicDevice *device, const DRequestPacket &packet)
{
    qDebug() << __FUNCTION__ << QString("id:%1, api:%2").arg(packet.id).arg(packet.api);

    MessagePacket sendPacket(packet.api, packet.id);
    sendPacket.setParams(packet.paramsObj);

    QStringList list;
    list << "SetCPCmd" << "SetCPLECmd";

    bool defaultValue = false;
    if (list.contains(packet.api)) {
        defaultValue = true;
    }

    bool isQueued = packet.paramsObj.value("isQueued").toBool(defaultValue);
    sendPacket.setIsQueue(isQueued);

    quint8 slaveIndex = static_cast<quint8>(packet.paramsObj.value("slaveIndex").toInt());
    sendPacket.setTargetType(packet.target, slaveIndex);

    if (device) {
        bool ok = device->sendCommand(sendPacket);
        return ok;
    }
    return false;
}

void MagicDevicePlugin::pQueuedCmdStop(MagicDevice *device, const DRequestPacket &packet)
{
    qDebug() << __FUNCTION__;

    MessagePacket p(packet.api, packet.id);
    bool isForceStop = packet.paramsObj.value("forceStop").toBool();
    if (isForceStop == true) {
        p.setCommand("QueuedCmdForceStop");
    }

    if (device) {
        device->sendCommand(p);
    }
}

bool MagicDevicePlugin::pSendCommand(MagicDevice *device, const DRequestPacket &packet)
{
    qDebug() << __FUNCTION__ << QString("id:%1, api:%2").arg(packet.id).arg(packet.api);

    MessagePacket p(packet.api, packet.id);
    p.setParams(packet.paramsObj);

    quint8 slaveIndex = static_cast<quint8>(packet.paramsObj.value("slaveIndex").toInt());
    p.setTargetType(packet.target, slaveIndex);

    if (device) {
        bool ok = device->sendCommand(p);
        return ok;
    }
    return false;
}

/* 处理box下载命令 */
void MagicDevicePlugin::handleDownloadCmd(MagicDevice *device, const DRequestPacket &packet)
{
    Q_UNUSED(device)

    if (packet.paramsObj.contains("code")) {
        QString code = packet.paramsObj.value("code").toString();
        //dobot_scratch.py
        QString fileName = packet.paramsObj.value("fileName").toString("temp");
        downloader->handleProgramRequest(fileName, code, packet.id);
    }
}

/* 接收 Device 消息 */
void MagicDevicePlugin::handleReceiveMessage_slot(quint64 id, QString cmd, int res, QJsonValue params)
{
#ifndef QT_NO_DEBUG
    qDebug().noquote() << __FUNCTION__
                       << QString("id:%1, cmd:%2, res:%3 params:").arg(id).arg(cmd).arg(res)
                       << params;
#endif

    MagicDevice *device = qobject_cast<MagicDevice *>(sender());
    ErrorType e = static_cast<ErrorType>(DEVICE_DLL_ERROR_BASE + res);

    if (cmd.isEmpty()) {
        qDebug() << "No cmd value. id:" << id;
    }

    if (m_requestPacketMap.contains(id)) {
        DRequestPacket requestPacket = m_requestPacketMap.take(id);
        DResultPacket resPacket(requestPacket);

        if (res == NoError) {
            emit pSendMessage_signal(PluginName, resPacket.getResultObj(params));
        } else {
            emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(e));
        }
    } else if (e == ERROR_DEVICE_LOST_CONNECTION) {
        DNotificationPacket notiPacket(device->getWebsocketPort());
        QJsonObject paramsObj;
        paramsObj.insert("portName", device->getPortName());
        QString method = "dobotlink.notification.lostConnection";
        QJsonObject notificationObj = notiPacket.getNotificationObj(method, paramsObj);

        emit pSendMessage_signal(PluginName, notificationObj);
    } else {
        qDebug() << "request packet is not found. id:" << id;
    }
}

void MagicDevicePlugin::handleCheckDeviceType_slot(quint64 id, QString cmd, int res, QJsonValue params)
{
    MagicDevice *device = qobject_cast<MagicDevice *>(sender());
    if (device == nullptr) {
        qDebug() << __FUNCTION__ << "error";
        return;
    }

    QString portName = device->getPortName();
    DeviceType type = device->getDeviceType();

    if (cmd == "ConnectDobot" and res == 0) {
        qDebug() << "device connected. portName:" << portName;
        return;
    }

    m_handlingSearchDeviceList.removeOne(portName);
    qDebug() << "check finish. portName:" << portName << "type:" << type;

    if (!m_preDeviceMap.contains(portName)) {
        qDebug() << "donot contains pre DeviceMap.";
        return;
    }

    qDebug() << "get check device type:" << id << "res:" << res << "cmd:" << cmd << params;
//    ErrorType e = static_cast<ErrorType>(DEVICE_DLL_ERROR_BASE + res);

    if (cmd.isEmpty() || res != 0) {
        device->disConnectDevice();
        device->deleteLater();
        m_preDeviceMap.remove(portName);
    } else if (cmd == "GetProductName") {
        if (res == NoError) {
            QString productName = params.toObject().value("productName").toString();

            if (type == DEVICE_MAGICIAN_LITE
                    and productName.compare("magicianlite", Qt::CaseInsensitive) != 0)
            {
                qDebug() << "don't match MagicianLite device.";
                m_preDeviceMap.remove(portName);
            }
            else if (type == DEVICE_MAGICBOX
                     and productName.compare("ExtController", Qt::CaseInsensitive) != 0)
            {
                qDebug() << "don't match MagicBox device.";
                m_preDeviceMap.remove(portName);
            }
        }
        device->disConnectDevice();
        device->deleteLater();
    }
    if (m_handlingSearchDeviceList.isEmpty()) {
        QJsonArray array = _getSearchResult();
        _sendResMessage(id, array);
        m_isSearchingDevices = false;
    }
}

void MagicDevicePlugin::onUdpReadyRead_slot()
{
    QByteArray m_data;
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QString deviceName = datagram.data();
        qDebug() << "[UDP BroadCast]get reply:" << deviceName;

        if (deviceName.startsWith("dobotM1_")) {
            QString address = datagram.senderAddress().toString();
            QRegExp rx("((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)");
            if (rx.indexIn(address) > -1) {
                QString deviceIp = rx.cap();
                m_m1deviceMap.insert(deviceIp, deviceName);
                qDebug() << "[UDP BroadCast]find a M1 Device, ip:" << deviceIp << "name:" << deviceName;

                QStringList list = deviceIp.split(".");
                list.removeLast();
                QString iphead = list.join(".");
                foreach (QString ip, getHostIpList()) {
                    if (ip.contains(iphead)) {
                        m_localIp = ip;
                        qDebug() << "verify localIp:" << m_localIp;
                        break;
                    }
                }
            }
        }
    }
}

void MagicDevicePlugin::onSearchTimeout_slot()
{
    if (!m_m1deviceMap.isEmpty()) {
        foreach (const QString &ip, m_m1deviceMap.keys()) {

            DeviceInfo info;
            info.portName = ip;
            info.description = m_m1deviceMap.value(ip);
            info.status = "unconnected";

            if (m_deviceMap.contains(ip)) {
                MagicDevice *device = m_deviceMap.value(ip);
                if (device) {
                    info.status = device->getConnectStatus();
                }
            }

            m_preDeviceMap.insert(ip, info);
        }
    }

    QJsonArray array = _getSearchResult();
    _sendResMessage(m_searchTimer->property("id").toInt(), array);
    m_isSearchingDevices = false;
}

void MagicDevicePlugin::_closeAllDevice()
{
    foreach (const QString &portName, m_deviceMap.keys()) {
        MagicDevice *device = m_deviceMap.value(portName);
        if (device) {
            device->disConnectDevice();
        }
    }
}

void MagicDevicePlugin::_sendResMessage(const quint64 id, const QJsonValue resValue)
{
    DRequestPacket requestPacket = m_requestPacketMap.take(id);
    DResultPacket resPacket(requestPacket);
    emit pSendMessage_signal(PluginName, resPacket.getResultObj(resValue));
}

void MagicDevicePlugin::_sendResMessage(const DRequestPacket &request, const QJsonValue resValue)
{
    DResultPacket resPacket(request);
    emit pSendMessage_signal(PluginName, resPacket.getResultObj(resValue));
}

void MagicDevicePlugin::_sendErrorMessage(const quint64 id, const ErrorType type)
{
    DRequestPacket requestPacket = m_requestPacketMap.take(id);
    DResultPacket resPacket(requestPacket);
    emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(type));
}

void MagicDevicePlugin::_sendErrorMessage(const DRequestPacket &request, const ErrorType type)
{
    DResultPacket resPacket(request);
    emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(type));
}

void MagicDevicePlugin::_broadcastForSearchM1()
{
    foreach (QString ip, getHostIpList()) {
        QString boradcastIP = m_ipAndBroadcastMap.value(ip);
        qDebug() << "[Info] localIP:" << ip << ", broadcastIP:" << boradcastIP;
        m_udpSocket->writeDatagram(BroadCastMessage, QHostAddress(boradcastIP), BroadCastPort);
    }

    m_searchTimer->start();
}

QStringList MagicDevicePlugin::getHostIpList()
{
    qDebug() << "[Info] localIP addresses:" << QNetworkInterface::allAddresses();

    m_ipAndBroadcastMap.clear();

    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        if (!interface.isValid()) {
            continue;
        }

        QNetworkInterface::InterfaceFlags flags = interface.flags();
        if (flags.testFlag(QNetworkInterface::IsUp)
                and flags.testFlag(QNetworkInterface::IsRunning)
                and !flags.testFlag(QNetworkInterface::IsLoopBack))
        {
            if (interface.humanReadableName().contains("VMware")
                    || interface.humanReadableName().contains("Loopback")
                    || interface.humanReadableName().contains("VirtualBox"))
            {
                continue;
            }

            foreach (const QNetworkAddressEntry &entry, interface.addressEntries())
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    QString ip = entry.ip().toString();
                    QString netmask = entry.netmask().toString();
                    QString broadcast = entry.broadcast().toString();

                    if (!broadcast.isEmpty()) {
                        m_ipAndBroadcastMap.insert(ip, broadcast);
                    }
                }
            }
        }
    }
    return m_ipAndBroadcastMap.keys();
}

/* SLOT */
void MagicDevicePlugin::handleDownloadFinished_slot(quint64 id, bool isOk, QJsonObject errorObj)
{
    DRequestPacket requestPacket = m_requestPacketMap.take(id);
    DResultPacket resPacket(requestPacket);
    if (isOk) {
        emit pSendMessage_signal(PluginName, resPacket.getResultObj());
    } else {
        emit pSendMessage_signal(PluginName, resPacket.setErrorObj(errorObj));
    }
}
