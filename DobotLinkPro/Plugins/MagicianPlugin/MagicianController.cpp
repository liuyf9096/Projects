#include "MagicianController.h"

#include <QJsonArray>
#include <QSerialPortInfo>
#include <QDebug>
#include "DobotDll.h"

DeviceInfo::DeviceInfo()
{
    id = -1;
    isConnected = false;
    status = "unknown";
}

MagicDeviceController::MagicDeviceController(QObject *parent) : QObject(parent)
{
    m_ConnectedDeviceCount = 0;

    checkStateTimer = new QTimer(this);
    checkStateTimer->setInterval(300);
    connect(checkStateTimer, &QTimer::timeout, this, &MagicDeviceController::handleCheckTimeout_slot);
    checkStateTimer->start();
}

bool MagicDeviceController::isDeviceConnected(QString portName)
{
    if (m_DevInfoMap.contains(portName)) {
        DeviceInfo info = m_DevInfoMap.value(portName);
        if (info.isConnected) {
            return true;
        }
        qDebug() << "Dobot status:" << info.status;
    }
    qDebug() << "! Dobot is not connected. portName:" << portName;
    return false;
}

/* 心跳检测判断设备连接状态 */
void MagicDeviceController::updataDeviceState()
{
    QStringList serialPortList;
    foreach (const auto &portInfo, QSerialPortInfo::availablePorts()) {
        serialPortList.append(portInfo.portName());
    }

    foreach (auto deviceInfo, m_DevInfoMap.values()) {
        if (deviceInfo.isConnected) {

            if (!serialPortList.contains(deviceInfo.portName)) {
                qDebug() << "Magician device:" << deviceInfo.portName << "lost connection.";

                quint16 wsPort = m_PortnamePortMap.value(deviceInfo.portName);
                deviceInfo.isConnected = false;
                deviceInfo.status = "unConnected";
                m_DevInfoMap.insert(deviceInfo.portName, deviceInfo);
                pDisConnectDobot(deviceInfo.portName);

                emit deviceDisconnected_signal(deviceInfo.portName, wsPort);
            }
        }
    }
}

void MagicDeviceController::disConnectDevices(quint16 port)
{
    QStringList portNameList = m_PortnamePortMap.keys(port);
    foreach (const QString &portName, portNameList) {
        pDisConnectDobot(portName);
    }
}

bool MagicDeviceController::isDeviceAvailable(QString portName)
{
    if (m_DevInfoMap.contains(portName)) {
        DeviceInfo info = m_DevInfoMap.value(portName);
        return info.isConnected;
    }
    return false;
}

QJsonObject MagicDeviceController::getCmdResultObj(const MagicianPacket &packet, int res, QJsonValue data)
{
    MagicianResPacket resPacket(packet.id, packet.port);

    if (res == DobotCommunicate_NoError) {
        if (data.isNull()) {
            resPacket.setResultObj(true);
        } else {
            resPacket.setResultObj(data);
        }
    } else {
        if (res == DobotCommunicate_BufferFull) {
            resPacket.setErrorObj(1, "Communicate Buffer Full.");
        } else if (res == DobotCommunicate_Timeout) {
            resPacket.setErrorObj(2, "Communicate Timeout.");
        } else if (res == DobotCommunicate_InvalidParams) {
            resPacket.setErrorObj(3, "Communicate Invalid Params.");
        } else if (res == DobotCommunicate_IsRuning) {
            resPacket.setErrorObj(4, "Communicate is running.");
        } else {
            resPacket.setErrorObj(20, "DobotCommunicate Unknow error.");
        }
    }
    return resPacket.getResultObj();
}

QJsonObject MagicDeviceController::getConnectResultObj(const MagicianPacket &packet, const int res)
{
    MagicianResPacket resPacket(packet.id, packet.port);

    if (res == DobotConnect_NoError) {
        resPacket.setResultObj(true);
    } else if (res == DobotConnect_NotFound) {
        resPacket.setErrorObj(10, "DobotConnect NotFound.");
    } else if (res == DobotConnect_Occupied) {
        resPacket.setErrorObj(11, "DobotConnect Occupied.");
    } else if (res == DobotConnect_AllReadyConnected) {
        resPacket.setErrorObj(12, "Magician is Already Connected.");
    } else if (res == DobotConnect_Unknown) {
        resPacket.setErrorObj(13, "DobotConnect Unknown error.");
    }
    return resPacket.getResultObj();
}

QJsonObject MagicDeviceController::pSetCmdTimeout(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    int cmdTimeout = packet.paramsObj.value("cmdTimeout").toInt();

    int res = SetCmdTimeout(info.id, cmdTimeout);
    return getCmdResultObj(packet, res);
}

/* 搜索 */
QJsonObject MagicDeviceController::pSearchDobot(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    QJsonObject resObj;
    QJsonArray deviceArray;

    char *devsChr = new char[2000]();
    int devCount = SearchDobot(devsChr, 1024);
    if (devCount <= 0) {
        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(81, "No Dobot was found :( ");
        return resPacket.getResultObj();
    }
    qDebug() << "find" << devCount << "Devices.";

    QString devsStr(devsChr);
    QStringList devList = devsStr.split(' ', QString::SkipEmptyParts);
    delete[] devsChr;
    qDebug() << devList;

    for (int i = 0; i < devList.count(); i++) {

        QString portName = devList.at(i);
        DeviceInfo info = m_DevInfoMap.value(portName);

        if (info.portName.isEmpty() || info.isConnected == false) {

            int devID = -1;
            int res = ConnectDobotSerial(portName, &devID);
            if (res == DobotConnect_NoError) {
                qDebug() << "ConnectDobotSerial:" << portName << "dev_id:" << devID;

                info.id = devID;

                if (packet.paramsObj.value("forDetails").toBool(true)) {

                    /* Get Name */
                    char deviceName[64];
                    int resGetName = GetDeviceName(devID, deviceName, sizeof(deviceName));
                    if (resGetName == DobotConnect_NoError) {
                        QString name(deviceName);
                        info.name = name;

                        /* Get SN */
                        char *snchar = new char[100];
                        int resGetSN = GetDeviceSN(devID, snchar, 100);
                        if (resGetSN == DobotConnect_NoError) {
                            info.SN = snchar;
                            info.status = "unConnected";
                        }
                    } else {
                        info.status = "Error";
                    }

                    DisconnectDobot(info.id);
                }
            } else if (res == DobotConnect_Occupied) {
                info.status = "Occupied";
            }

            info.portName = portName;
            m_DevInfoMap.insert(portName, info);
        }

        if (info.isConnected) {
            info.status = "Connected";
        }
        QJsonObject deviceInfoObj;
        deviceInfoObj.insert("portName", portName);
        deviceInfoObj.insert("name", info.name);
        deviceInfoObj.insert("sn", info.SN);
        deviceInfoObj.insert("status", info.status);
        deviceArray.append(deviceInfoObj);
    }
    resObj.insert("array", deviceArray);
    return getCmdResultObj(packet, 0, deviceArray);
}

/* 连接 */
QJsonObject MagicDeviceController::pConnectDobot(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__ << packet.portName << packet.port;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    int devID = -1;
#if 0
    int res = ConnectDobot(portName.toLatin1().data(), 115200, nullptr, nullptr, &devID);
    if (res == DobotConnect_NoError) {
        info.isConnected = true;
        info.id = devID;
        m_DevIDMap.insert(portName, info);

        SetQueuedCmdStartExec(devID);

        qDebug() << "magician connected id:" << devID << "portName:" << portName;
    }

    return getConnectResultObj(res);
#else

    int res = ConnectDobotSerial(packet.portName, &devID);
    if (res == DobotConnect_NoError) {
        info.portName = packet.portName;
        info.isConnected = true;
        info.id = devID;
        m_DevInfoMap.insert(packet.portName, info);
        m_PortnamePortMap.insert(info.portName, packet.port);

        m_ConnectedDeviceCount++;

        if (packet.paramsObj.value("queueStart").toBool(true)) {
            SetQueuedCmdStartExec(devID);
        }

        /* 设置超时时间 */
        SetCmdTimeout(devID, 3500);

        qDebug() << "magician connected id:" << info.id << "portName:" << packet.portName;
    }

    return getConnectResultObj(packet, res);
#endif
}

/* 1.3.3 断开连接 */
QJsonObject MagicDeviceController::pDisConnectDobot(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);
    if (info.isConnected == false) {
        qDebug() << "Magician already disconnected";
        m_ConnectedDeviceCount = _getConnecteDeviceCount();
        return getCmdResultObj(packet);
    }

    if (packet.paramsObj.value("queueStop").toBool(true)) {
        SetQueuedCmdStopExec(info.id);
    }
    if (packet.paramsObj.value("queueClear").toBool(true)) {
        SetQueuedCmdClear(info.id);
    }

    int res = DisconnectDobot(info.id);
    if (res == DobotConnect_NoError) {

        info.isConnected = false;
        info.status = "unConnected";
        m_DevInfoMap.insert(packet.portName, info);

        m_ConnectedDeviceCount--;
        m_PortnamePortMap.remove(packet.portName);

        qDebug() << "magician disConnected id:" << info.id << "portName:" << packet.portName;
    }

    /* 如果留有定时器，使它停止，并删除 */
    foreach (auto wait, m_waitFinishMap.keys(info.id)) {
        wait->stopWaiting();
        m_waitFinishMap.remove(wait);
        wait->deleteLater();
    }

    return getCmdResultObj(packet, res);
}

void MagicDeviceController::pDisConnectDobot(const QString &portName)
{
    qDebug() << "pDisConnectDobot portName:" << portName;
    if (!m_DevInfoMap.contains(portName)) {
        qDebug() << "portName was not register";
        return;
    }
    DeviceInfo info = m_DevInfoMap.value(portName);

    if (info.isConnected == false) {
        qDebug() << "magician already disconnected";
        m_ConnectedDeviceCount = _getConnecteDeviceCount();
        return;
    }

    SetQueuedCmdStopExec(info.id);
    SetQueuedCmdClear(info.id);

    int res = DisconnectDobot(info.id);
    if (res == DobotConnect_NoError) {

        info.isConnected = false;
        info.status = "unConnected";
        m_DevInfoMap.insert(portName, info);

        m_PortnamePortMap.remove(portName);
        m_ConnectedDeviceCount--;

        qDebug() << "magician disconnected id:" << info.id << "portName:" << portName;
    }

    /* 如果留有定时器，使它停止，并删除 */
    foreach (auto wait, m_waitFinishMap.keys(info.id)) {
        wait->stopWaiting();
        m_waitFinishMap.remove(wait);
        wait->deleteLater();
    }
}

/* 1.4 指令队列控制 */
QJsonObject MagicDeviceController::pSetQueuedCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    int res = 0;
    QString type = packet.paramsObj.value("type").toString();
    if (type == "Start") {
        res = SetQueuedCmdStartExec(info.id);
    } else if (type == "Stop") {
        res = SetQueuedCmdStopExec(info.id);
    } else if (type == "ForceStop") {
        res = SetQueuedCmdForceStopExec(info.id);
    } else if (type == "Clear") {
        res = SetQueuedCmdClear(info.id);
    } else {
        res = DobotCommunicate_InvalidParams;
    }

    return getCmdResultObj(packet, res);
}

/* 1.5 设备信息 */
//![1.5.1]
QJsonObject MagicDeviceController::pSetDeviceSN(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QString devSN = packet.paramsObj.value("DeviceSN").toString();

    int res = SetDeviceSN(info.id, devSN.toLatin1().data());
    return getCmdResultObj(packet, res);
}

//![1.5.2]
QJsonObject MagicDeviceController::pGetDeviceSN(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    char devSN[64];

    int res = GetDeviceSN(info.id, devSN, sizeof(devSN));
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("DeviceSN", devSN);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.5.3]
QJsonObject MagicDeviceController::pSetDeviceName(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QString devName = packet.paramsObj.value("deviceName").toString();

    int res = SetDeviceName(info.id, devName.toLatin1().data());
    return getCmdResultObj(packet, res);
}

//![1.5.4]
QJsonObject MagicDeviceController::pGetDeviceName(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    char deviceName[64];

    int res = GetDeviceName(info.id, deviceName, sizeof(deviceName));
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("deviceName", deviceName);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.5.5]
QJsonObject MagicDeviceController::pGetDeviceVersion(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    quint8 majorVersion, minorVersion, revision;

    int res = GetDeviceVersion(info.id, &majorVersion, &minorVersion, &revision);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("majorVersion", majorVersion);
        paramsObj.insert("minorVersion", minorVersion);
        paramsObj.insert("revision", revision);
//        paramsObj.insert("hwVersion", hwVersion);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.5.6]
QJsonObject MagicDeviceController::pSetDeviceWithL(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool isEnable = packet.paramsObj.value("isEnable").toBool(true);
    quint8 version = static_cast<quint8>(packet.paramsObj.value("version").toInt(1));

    int res = SetDeviceWithL(info.id, version, isEnable);
    return getCmdResultObj(packet, res);
}

//![1.5.7]
QJsonObject MagicDeviceController::pGetDeviceWithL(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isEnable;

    int res = GetDeviceWithL(info.id, &isEnable);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("isEnable", isEnable);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.5.8]
QJsonObject MagicDeviceController::pGetDeviceTime(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    quint32 deviceTime;

    int res = GetDeviceTime(info.id, &deviceTime);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("deviceTime", static_cast<double>(deviceTime));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

/* 1.6 实时位姿 */
QJsonObject MagicDeviceController::pGetPose(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    Pose poseParams;

    int res = GetPose(info.id, &poseParams);
    if (res == DobotCommunicate_NoError) {
        info.posX = static_cast<double>(poseParams.x);
        info.posY = static_cast<double>(poseParams.y);
        info.posZ = static_cast<double>(poseParams.z);
        paramsObj.insert("x", info.posX);
        paramsObj.insert("y", info.posY);
        paramsObj.insert("z", info.posZ);
        paramsObj.insert("r", static_cast<double>(poseParams.r));

#if 0
        QJsonObject jointAngleObj;
        for (int i = 0; i < 4; ++i) {
            QString name = QString("a%1").arg(i);
            jointAngleObj.insert("a0", static_cast<double>(poseParams.jointAngle[i]));
        }
        paramsObj.insert("jointAngle", jointAngleObj);
#else
        QJsonArray jointAngleArr;
        info.baseArc = static_cast<double>(poseParams.jointAngle[0]);
        info.bigArmArc = static_cast<double>(poseParams.jointAngle[1]);
        info.littleArmArc = static_cast<double>(poseParams.jointAngle[2]);

        for (int i=0; i<4; i++) {
            jointAngleArr.append(static_cast<double>(poseParams.jointAngle[i]));
        }
        paramsObj.insert("jointAngle", jointAngleArr);
#endif
        m_DevInfoMap.insert(packet.portName, info);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

QJsonObject MagicDeviceController::pGetPoseL(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    float l;

    int res = GetPoseL(info.id, &l);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("l", static_cast<double>(l));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

/* 1.7 报警功能 */
QJsonObject MagicDeviceController::pGetAlarmsState(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    quint8 alarmsState[16];
    quint32 len;

    int res = GetAlarmsState(info.id, alarmsState, &len, 200);
    if (res == DobotCommunicate_NoError) {
        QJsonArray arr;
        for (uint i=0; i<sizeof (alarmsState); i++) {
            arr.append(alarmsState[i]);
        }
        paramsObj.insert("status", arr);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

QJsonObject MagicDeviceController::pClearAllAlarmsState(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    int res = ClearAllAlarmsState(info.id);
    if (res == DobotCommunicate_NoError) {
        res = ResetPose(info.id, false, 0, 0);
        res = SetQueuedCmdStartExec(info.id);
    }

    return getCmdResultObj(packet, res);
}

/* 1.8 回零功能 */
QJsonObject MagicDeviceController::pSetHOMECmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    HOMECmd homeParams;
    quint64 queuedCmdIndex = 0;

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetHOMECmd(info.id, &homeParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(5000, 20000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }

    return getCmdResultObj(packet, res);
}

QJsonObject MagicDeviceController::pSetHOMEParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    HOMEParams homeParams;
    quint64 queuedCmdIndex = 0;

    homeParams.x = static_cast<float>(packet.paramsObj.value("x").toDouble());
    homeParams.y = static_cast<float>(packet.paramsObj.value("y").toDouble());
    homeParams.z = static_cast<float>(packet.paramsObj.value("z").toDouble());
    homeParams.r = static_cast<float>(packet.paramsObj.value("r").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetHOMEParams(info.id, &homeParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

QJsonObject MagicDeviceController::pGetHOMEParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    HOMEParams homeParams;

    int res = GetHOMEParams(info.id, &homeParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("x", static_cast<double>(homeParams.x));
        paramsObj.insert("y", static_cast<double>(homeParams.y));
        paramsObj.insert("z", static_cast<double>(homeParams.z));
        paramsObj.insert("r", static_cast<double>(homeParams.r));
    }
    return getCmdResultObj(packet, res, paramsObj);
}


/* 1.9 HHT 手持示教器 (1.9.1~1.9.5) */
//![1.9.1]
QJsonObject MagicDeviceController::pSetHHTTrigMode(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    HHTTrigMode hhtParams;

    int trigInt = packet.paramsObj.value("hhtTrigMode").toInt();
    hhtParams = static_cast<HHTTrigMode>(trigInt);

    int res = SetHHTTrigMode(info.id, hhtParams);
    return getCmdResultObj(packet, res);
}

//![1.9.2]
QJsonObject MagicDeviceController::pGetHHTTrigMode(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    HHTTrigMode hhtTrigMode;

    int res = GetHHTTrigMode(info.id, &hhtTrigMode);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("HHTTrigMode", static_cast<int>(hhtTrigMode));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.9.3]
QJsonObject MagicDeviceController::pSetHHTTrigOutputEnabled(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool isEnabled = packet.paramsObj.value("isEnabled").toInt();

    int res = SetHHTTrigOutputEnabled(info.id, isEnabled);
    return getCmdResultObj(packet, res);
}

//![1.9.4]
QJsonObject MagicDeviceController::pGetHHTTrigOutputEnabled(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isEnabled;

    int res = GetHHTTrigOutputEnabled(info.id, &isEnabled);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("isEnabled", isEnabled);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.9.5]
QJsonObject MagicDeviceController::pGetHHTTrigOutput(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isTriggered;

    int res = GetHHTTrigOutput(info.id, &isTriggered);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("isTriggered", isTriggered);
    }
    return getCmdResultObj(packet, res, paramsObj);
}


/* 1.10 末端执行器 (1.10.1 ~ 1.10.8) */
//![1.10.1]
QJsonObject MagicDeviceController::pSetEndEffectorParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    EndEffectorParams effectorParams;
    quint64 queuedCmdIndex = 0;

    effectorParams.xBias = static_cast<float>(packet.paramsObj.value("xBias").toDouble());
    effectorParams.yBias = static_cast<float>(packet.paramsObj.value("yBias").toDouble());
    effectorParams.zBias = static_cast<float>(packet.paramsObj.value("zBias").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetEndEffectorParams(info.id, &effectorParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.10.2]
QJsonObject MagicDeviceController::pGetEndEffectorParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    EndEffectorParams effectorParams;

    int res = GetEndEffectorParams(info.id, &effectorParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("xBias", static_cast<double>(effectorParams.xBias));
        paramsObj.insert("yBias", static_cast<double>(effectorParams.yBias));
        paramsObj.insert("zBias", static_cast<double>(effectorParams.zBias));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.10.3]
QJsonObject MagicDeviceController::pSetEndEffectorLaser(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool enableCtrl = packet.paramsObj.value("enableCtrl").toBool();
    bool on = packet.paramsObj.value("on").toBool();
    quint64 queuedCmdIndex = 0;

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetEndEffectorLaser(info.id, enableCtrl, on, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.10.4]
QJsonObject MagicDeviceController::pGetEndEffectorLaser(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isCtrlEnabled, isOn;

    int res = GetEndEffectorLaser(info.id, &isCtrlEnabled, &isOn);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("isCtrlEnabled", isCtrlEnabled);
        paramsObj.insert("isOn", isOn);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.10.5]
QJsonObject MagicDeviceController::pSetEndEffectorSuctionCup(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool enableCtrl = packet.paramsObj.value("enableCtrl").toBool();
    bool suck = packet.paramsObj.value("suck").toBool();
    quint64 queuedCmdIndex = 0;

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetEndEffectorSuctionCup(info.id, enableCtrl, suck, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.10.6]
QJsonObject MagicDeviceController::pGetEndEffectorSuctionCup(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isCtrlEnabled, isSucked;

    int res = GetEndEffectorSuctionCup(info.id, &isCtrlEnabled, &isSucked);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("isCtrlEnabled", isCtrlEnabled);
        paramsObj.insert("isSucked", isSucked);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.10.7]
QJsonObject MagicDeviceController::pSetEndEffectorGripper(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool enableCtrl = packet.paramsObj.value("enableCtrl").toBool();
    bool grip = packet.paramsObj.value("grip").toBool();
    quint64 queuedCmdIndex = 0;

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetEndEffectorGripper(info.id, enableCtrl, grip, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.10.8]
QJsonObject MagicDeviceController::pGetEndEffectorGripper(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    bool isCtrlEnabled, isGripped;

    int res = GetEndEffectorGripper(info.id, &isCtrlEnabled, &isGripped);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("CtrlEnabled", isCtrlEnabled);
        paramsObj.insert("Gripped", isGripped);
    }
    return getCmdResultObj(packet, res, paramsObj);
}


/* JOG 功能 (1.11.1 ~ 1.11.9) */
//![1.11.1]
QJsonObject MagicDeviceController::pSetJOGJointParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonArray velocityArr = packet.paramsObj.value("velocity").toArray();
    QJsonArray accelerationArr = packet.paramsObj.value("acceleration").toArray();

    JOGJointParams jogParams;
    quint64 queuedCmdIndex = 0;

    for (int i=0; i<velocityArr.count(); i++) {
        jogParams.velocity[i] = static_cast<float>(velocityArr.at(i).toDouble());
    }
    for (int i=0; i<accelerationArr.count(); i++) {
        jogParams.acceleration[i] = static_cast<float>(accelerationArr.at(i).toDouble());
    }

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetJOGJointParams(info.id, &jogParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.11.2]
QJsonObject MagicDeviceController::pGetJOGJointParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    JOGJointParams jogParams;

    int res = GetJOGJointParams(info.id, &jogParams);
    if (res == DobotCommunicate_NoError) {

        QJsonArray velocityArr, accelerationArr;
        for (int i=0; i<4; i++) {
            velocityArr.insert(i, static_cast<double>(jogParams.velocity[i]));
            accelerationArr.insert(i, static_cast<double>(jogParams.acceleration[i]));
        }
        paramsObj.insert("velocity", velocityArr);
        paramsObj.insert("acceleration", accelerationArr);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.11.3]
QJsonObject MagicDeviceController::pSetJOGCoordinateParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonArray velocityArr = packet.paramsObj.value("velocity").toArray();
    QJsonArray accelerationArr = packet.paramsObj.value("acceleration").toArray();

    JOGCoordinateParams jogParams;
    quint64 queuedCmdIndex = 0;

    for (int i=0; i<velocityArr.count(); i++) {
        jogParams.velocity[i] = static_cast<float>(velocityArr.at(i).toDouble());
    }
    for (int i=0; i<accelerationArr.count(); i++) {
        jogParams.acceleration[i] = static_cast<float>(accelerationArr.at(i).toDouble());
    }

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetJOGCoordinateParams(info.id, &jogParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.11.4]
QJsonObject MagicDeviceController::pGetJOGCoordinateParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    JOGCoordinateParams jogParams;

    int res = GetJOGCoordinateParams(info.id, &jogParams);
    if (res == DobotCommunicate_NoError) {

        QJsonArray velocityArr, accelerationArr;
        for (int i=0; i<4; i++) {
            velocityArr.insert(i, static_cast<double>(jogParams.velocity[i]));
            accelerationArr.insert(i, static_cast<double>(jogParams.acceleration[i]));
        }
        paramsObj.insert("velocity", velocityArr);
        paramsObj.insert("acceleration", accelerationArr);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.11.5]
QJsonObject MagicDeviceController::pSetJOGLParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    JOGLParams jogParams;
    quint64 queuedCmdIndex = 0;

    jogParams.velocity = static_cast<float>(packet.paramsObj.value("velocity").toDouble());
    jogParams.acceleration = static_cast<float>(packet.paramsObj.value("acceleration").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetJOGLParams(info.id, &jogParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.11.6]
QJsonObject MagicDeviceController::pGetJOGLParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    JOGLParams jogParams;

    int res = GetJOGLParams(info.id, &jogParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("velocity", static_cast<double>(jogParams.velocity));
        paramsObj.insert("acceleration", static_cast<double>(jogParams.acceleration));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.11.7]
QJsonObject MagicDeviceController::pSetJOGCommonParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    JOGCommonParams jogParams;
    quint64 queuedCmdIndex = 0;

    jogParams.velocityRatio = static_cast<float>(packet.paramsObj.value("velocityRatio").toDouble());
    jogParams.accelerationRatio = static_cast<float>(packet.paramsObj.value("accelerationRatio").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetJOGCommonParams(info.id, &jogParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.11.8]
QJsonObject MagicDeviceController::pGetJOGCommonParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    JOGCommonParams jogParams;

    int res = GetJOGCommonParams(info.id, &jogParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("velocityRatio", static_cast<double>(jogParams.velocityRatio));
        paramsObj.insert("accelerationRatio", static_cast<double>(jogParams.accelerationRatio));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.11.9]
QJsonObject MagicDeviceController::pSetJOGCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    JOGCmd jogcmd;
    jogcmd.isJoint = static_cast<quint8>(packet.paramsObj.value("isJoint").toInt());
    jogcmd.cmd = static_cast<quint8>(packet.paramsObj.value("cmd").toInt());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetJOGCmd(info.id, &jogcmd, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}


/* PTP 功能 (1.12.1 ~ 1.12.13) */
//![1.12.1]
QJsonObject MagicDeviceController::pSetPTPJointParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPJointParams ptpParams;
    quint64 queuedCmdIndex = 0;

    QJsonValue velocityValue = packet.paramsObj.value("velocity");
    if (velocityValue.isObject()) {
        QJsonObject velocityObj = packet.paramsObj.value("velocity").toObject();
        ptpParams.velocity[0] = static_cast<float>(velocityObj.value("v0").toDouble());
        ptpParams.velocity[1] = static_cast<float>(velocityObj.value("v1").toDouble());
        ptpParams.velocity[2] = static_cast<float>(velocityObj.value("v2").toDouble());
        ptpParams.velocity[3] = static_cast<float>(velocityObj.value("v3").toDouble());
    } else if (velocityValue.isArray()) {
        QJsonArray velocityArr = packet.paramsObj.value("velocity").toArray();
        for (int i=0; i<velocityArr.count(); i++) {
            ptpParams.velocity[i] = static_cast<float>(velocityArr.at(i).toDouble());
        }
    }

    QJsonValue accelerationValue = packet.paramsObj.value("acceleration");
    if (accelerationValue.isObject()) {
        QJsonObject accelerationObj = packet.paramsObj.value("acceleration").toObject();
        ptpParams.acceleration[0] = static_cast<float>(accelerationObj.value("a0").toDouble());
        ptpParams.acceleration[1] = static_cast<float>(accelerationObj.value("a1").toDouble());
        ptpParams.acceleration[2] = static_cast<float>(accelerationObj.value("a2").toDouble());
        ptpParams.acceleration[3] = static_cast<float>(accelerationObj.value("a3").toDouble());
    } else if (accelerationValue.isArray()) {
        QJsonArray accelerationArr = packet.paramsObj.value("acceleration").toArray();
        for (int i=0; i<accelerationArr.count(); i++) {
            ptpParams.acceleration[i] = static_cast<float>(accelerationArr.at(i).toDouble());
        }
    }

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPJointParams(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.2]
QJsonObject MagicDeviceController::pGetPTPJointParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPJointParams ptpParams;

    int res = GetPTPJointParams(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        QJsonArray velocityArr;
        for (int i=0; i<4; i++) {
            velocityArr.append(static_cast<double>(ptpParams.velocity[i]));
        }
        paramsObj.insert("velocity", velocityArr);

        QJsonArray accelerationArr;
        for (int i=0; i < 4; i++) {
            accelerationArr.append(static_cast<double>(ptpParams.acceleration[i]));
        }
        paramsObj.insert("acceleration", accelerationArr);
    }

    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.3]
QJsonObject MagicDeviceController::pSetPTPCoordinateParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPCoordinateParams ptpParams;
    quint64 queuedCmdIndex = 0;

    ptpParams.xyzVelocity = static_cast<float>(packet.paramsObj.value("xyzVelocity").toDouble()); //X,Y,Z 3轴坐标轴速度
    ptpParams.rVelocity = static_cast<float>(packet.paramsObj.value("rVelocity").toDouble());     //末端 R 轴速度
    ptpParams.xyzAcceleration = static_cast<float>(packet.paramsObj.value("xyzAcceleration").toDouble()); //X,Y,Z 3轴坐标轴加速度
    ptpParams.rAcceleration = static_cast<float>(packet.paramsObj.value("rAcceleration").toDouble());     //末端 R 轴加速度

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPCoordinateParams(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.4]
QJsonObject MagicDeviceController::pGetPTPCoordinateParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPCoordinateParams ptpParams;

    int res = GetPTPCoordinateParams(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("xyzVelocity", static_cast<double>(ptpParams.xyzVelocity));
        paramsObj.insert("rVelocity", static_cast<double>(ptpParams.rVelocity));
        paramsObj.insert("xyzAcceleration", static_cast<double>(ptpParams.xyzAcceleration));
        paramsObj.insert("rAccleration", static_cast<double>(ptpParams.rAcceleration));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.5]
QJsonObject MagicDeviceController::pSetPTPJumpParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPJumpParams ptpParams;
    quint64 queuedCmdIndex = 0;

    ptpParams.jumpHeight = static_cast<float>(packet.paramsObj.value("jumpHeight").toDouble());   //抬升高度
    ptpParams.zLimit = static_cast<float>(packet.paramsObj.value("zLimit").toDouble());           //最大抬升高度

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPJumpParams(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.6]
QJsonObject MagicDeviceController::pGetPTPJumpParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPJumpParams ptpParams;

    int res = GetPTPJumpParams(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("jumpHeight", static_cast<double>(ptpParams.jumpHeight));
        paramsObj.insert("zLimit", static_cast<double>(ptpParams.zLimit));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.7]
QJsonObject MagicDeviceController::pSetPTPJump2Params(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPJump2Params ptpParams;
    quint64 queuedCmdIndex = 0;

    ptpParams.startJumpHeight = static_cast<float>(packet.paramsObj.value("startJumpHeight").toDouble()); //起始点抬升高度
    ptpParams.endJumpHeight = static_cast<float>(packet.paramsObj.value("endJumpHeight").toDouble());     //结束点抬升高度
    ptpParams.zLimit = static_cast<float>(packet.paramsObj.value("zLimit").toDouble());                   //最大抬升高度

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPJump2Params(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.8]
QJsonObject MagicDeviceController::pGetPTPJump2Params(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPJump2Params ptpParams;

    int res = GetPTPJump2Params(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("startJumpHeight", static_cast<double>(ptpParams.startJumpHeight));
        paramsObj.insert("endJumpHeight", static_cast<double>(ptpParams.endJumpHeight));
        paramsObj.insert("zLimit", static_cast<double>(ptpParams.zLimit));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.9]
QJsonObject MagicDeviceController::pSetPTPLParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPLParams ptpParams;
    quint64 queuedCmdIndex = 0;

    ptpParams.velocity = static_cast<float>(packet.paramsObj.value("velocity").toDouble());           //滑轨速度
    ptpParams.acceleration = static_cast<float>(packet.paramsObj.value("acceleration").toDouble());   //滑轨加速度

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPLParams(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.10]
QJsonObject MagicDeviceController::pGetPTPLParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPLParams ptpParams;

    int res = GetPTPLParams(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("velocity", static_cast<double>(ptpParams.velocity));
        paramsObj.insert("acceleration", static_cast<double>(ptpParams.acceleration));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.11]
QJsonObject MagicDeviceController::pSetPTPCommonParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    PTPCommonParams ptpParams;
    quint64 queuedCmdIndex = 0;

    ptpParams.velocityRatio = static_cast<float>(packet.paramsObj.value("velocityRatio").toDouble());
    ptpParams.accelerationRatio = static_cast<float>(packet.paramsObj.value("accelerationRatio").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetPTPCommonParams(info.id, &ptpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.12.12]
QJsonObject MagicDeviceController::pGetPTPCommonParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    PTPCommonParams ptpParams;

    int res = GetPTPCommonParams(info.id, &ptpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("velocityRatio", static_cast<double>(ptpParams.velocityRatio));
        paramsObj.insert("accelerationRatio", static_cast<double>(ptpParams.accelerationRatio));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.12.13]
QJsonObject MagicDeviceController::pSetPTPCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    PTPCmd ptpParams;
    ptpParams.ptpMode = static_cast<quint8>(packet.paramsObj.value("ptpMode").toInt());
    ptpParams.x = static_cast<float>(packet.paramsObj.value("x").toDouble());
    ptpParams.y = static_cast<float>(packet.paramsObj.value("y").toDouble());
    ptpParams.z = static_cast<float>(packet.paramsObj.value("z").toDouble());
    ptpParams.r = static_cast<float>(packet.paramsObj.value("r").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPCmd(info.id, &ptpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}

/* 后增加 设置R角度 */
QJsonObject MagicDeviceController::pSetRCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    PTPCmd ptpParams;
    ptpParams.ptpMode = static_cast<quint8>(packet.paramsObj.value("ptpMode").toInt(1));
    if (ptpParams.ptpMode == 4) {
        ptpParams.x = static_cast<float>(info.baseArc);
        ptpParams.y = static_cast<float>(info.bigArmArc);
        ptpParams.z = static_cast<float>(info.littleArmArc);
    } else {
        ptpParams.x = static_cast<float>(info.posX);
        ptpParams.y = static_cast<float>(info.posY);
        ptpParams.z = static_cast<float>(info.posZ);
    }

    ptpParams.r = static_cast<float>(packet.paramsObj.value("r").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPCmd(info.id, &ptpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}

//![1.12.14]
QJsonObject MagicDeviceController::pSetPTPPOCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject PTPCmdObj;
    PTPCmd ptpParams;

    if (packet.paramsObj.contains("ptpCmd")) {
        PTPCmdObj = packet.paramsObj.value("ptpCmd").toObject();

        ptpParams.ptpMode = static_cast<quint8>(PTPCmdObj.value("ptpMode").toInt());
        ptpParams.x = static_cast<float>(PTPCmdObj.value("x").toDouble());
        ptpParams.y = static_cast<float>(PTPCmdObj.value("y").toDouble());
        ptpParams.z = static_cast<float>(PTPCmdObj.value("z").toDouble());
        ptpParams.r = static_cast<float>(PTPCmdObj.value("r").toDouble());
    }

    ParallelOutputCmd outputCmd[100];
    int count = 0;
    if (packet.paramsObj.contains("parallelOutputCmd")) {
        QJsonValue parallelOutputCmdValue = packet.paramsObj.value("parallelOutputCmd");
        if (parallelOutputCmdValue.isArray()) {
            QJsonArray parallelOutputCmdArray = parallelOutputCmdValue.toArray();
            count = parallelOutputCmdArray.count();
            if (count > 0) {
                for (int i = 0; i < count; i++) {
                    QJsonObject parallelOutputCmdObj = parallelOutputCmdArray.at(i).toObject();
                    outputCmd[i].ratio = static_cast<quint8>(parallelOutputCmdObj.value("ratio").toInt());
                    outputCmd[i].address = static_cast<quint16>(parallelOutputCmdObj.value("address").toInt());
                    outputCmd[i].level = static_cast<quint8>(parallelOutputCmdObj.value("level").toInt());
                }
            }
        }
    }

    quint64 queuedCmdIndex = 0;
    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPPOCmd(info.id, &ptpParams, outputCmd, count, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}

//![1.12.15]
QJsonObject MagicDeviceController::pSetPTPWithLCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    PTPWithLCmd ptpParams;
    ptpParams.ptpMode = static_cast<quint8>(packet.paramsObj.value("ptpMode").toInt());
    ptpParams.x = static_cast<float>(packet.paramsObj.value("x").toDouble());
    ptpParams.y = static_cast<float>(packet.paramsObj.value("y").toDouble());
    ptpParams.z = static_cast<float>(packet.paramsObj.value("z").toDouble());
    ptpParams.r = static_cast<float>(packet.paramsObj.value("r").toDouble());
    ptpParams.l = static_cast<float>(packet.paramsObj.value("l").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPWithLCmd(info.id, &ptpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}

QJsonObject MagicDeviceController::pSetLCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    PTPWithLCmd ptpParams;
    ptpParams.ptpMode = static_cast<quint8>(packet.paramsObj.value("ptpMode").toInt(1));
    if (ptpParams.ptpMode == 4) {
        ptpParams.x = static_cast<float>(info.baseArc);
        ptpParams.y = static_cast<float>(info.bigArmArc);
        ptpParams.z = static_cast<float>(info.littleArmArc);
    } else {
        ptpParams.x = static_cast<float>(info.posX);
        ptpParams.y = static_cast<float>(info.posY);
        ptpParams.z = static_cast<float>(info.posZ);
    }

    ptpParams.l = static_cast<float>(packet.paramsObj.value("l").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPWithLCmd(info.id, &ptpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}

//![1.12.16]
QJsonObject MagicDeviceController::pSetPTPPOWithLCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject PTPCmdObj;
    PTPWithLCmd ptpParams;

    if (packet.paramsObj.contains("ptpCmd")) {
//        PTPCmdObj = packet.paramsObj.value("PTPCmdObj").toObject();
        PTPCmdObj = packet.paramsObj.value("ptpCmd").toObject();

        ptpParams.ptpMode = static_cast<quint8>(PTPCmdObj.value("ptpMode").toInt());
        ptpParams.x = static_cast<float>(PTPCmdObj.value("x").toDouble());
        ptpParams.y = static_cast<float>(PTPCmdObj.value("y").toDouble());
        ptpParams.z = static_cast<float>(PTPCmdObj.value("z").toDouble());
        ptpParams.r = static_cast<float>(PTPCmdObj.value("r").toDouble());
        ptpParams.l = static_cast<float>(PTPCmdObj.value("l").toDouble());
    }

    QJsonObject ParallelOutputCmdObj;
    ParallelOutputCmd outputCmd;

    if (packet.paramsObj.contains("parallelOutputCmd")) {
        ParallelOutputCmdObj = packet.paramsObj.value("parallelOutputCmd").toObject();

        outputCmd.ratio = static_cast<quint8>(ParallelOutputCmdObj.value("ratio").toInt());
        outputCmd.address = static_cast<quint16>(ParallelOutputCmdObj.value("address").toInt());
        outputCmd.level = static_cast<quint8>(ParallelOutputCmdObj.value("level").toInt());
    }

    quint64 queuedCmdIndex = 0;
    int parallelCmdCount = packet.paramsObj.value("parallelCmdCount").toInt();

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(true);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetPTPPOWithLCmd(info.id, &ptpParams, &outputCmd, parallelCmdCount, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 10000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
    return getCmdResultObj(packet, res);
}


/* CP 功能 (1.13.1 ~ 1.13.4) */
//![1.13.1]
QJsonObject MagicDeviceController::pSetCPParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    CPParams cpParams;
    quint64 queuedCmdIndex = 0;

    cpParams.planAcc = static_cast<float>(packet.paramsObj.value("planAcc").toDouble());
    cpParams.juncitionVel = static_cast<float>(packet.paramsObj.value("juncitionVel").toDouble());
    cpParams.acc = static_cast<float>(packet.paramsObj.value("acc").toDouble());
    cpParams.period = static_cast<float>(packet.paramsObj.value("period").toDouble());
    cpParams.realTimeTrack = static_cast<quint8>(packet.paramsObj.value("realTimeTrack").toInt());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetCPParams(info.id, &cpParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.13.2]
QJsonObject MagicDeviceController::pGetCPParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    CPParams cpParams;

    int res = GetCPParams(info.id, &cpParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("planAcc", static_cast<double>(cpParams.planAcc));
        paramsObj.insert("juncitionVel", static_cast<double>(cpParams.juncitionVel));
        paramsObj.insert("acc", static_cast<double>(cpParams.acc));
        paramsObj.insert("period", static_cast<double>(cpParams.period));
        paramsObj.insert("realTimeTrack", cpParams.realTimeTrack);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.13.3]
QJsonObject MagicDeviceController::pSetCPCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    CPCmd cpParams;
    cpParams.cpMode = static_cast<quint8>(packet.paramsObj.value("cpMode").toInt());
    cpParams.x = static_cast<float>(packet.paramsObj.value("x").toDouble());
    cpParams.y = static_cast<float>(packet.paramsObj.value("y").toDouble());
    cpParams.z = static_cast<float>(packet.paramsObj.value("z").toDouble());
    cpParams.velocity = static_cast<float>(packet.paramsObj.value("velocity").toDouble());
    cpParams.power = static_cast<float>(packet.paramsObj.value("power").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(false);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetCPCmd(info.id, &cpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 5000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }

    return getCmdResultObj(packet, res);
}

//![1.13.4]
QJsonObject MagicDeviceController::pSetCPLECmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    CPCmd cpParams;
    cpParams.cpMode = static_cast<quint8>(packet.paramsObj.value("cpMode").toInt());
    cpParams.x = static_cast<float>(packet.paramsObj.value("x").toDouble());
    cpParams.y = static_cast<float>(packet.paramsObj.value("y").toDouble());
    cpParams.z = static_cast<float>(packet.paramsObj.value("z").toDouble());
    cpParams.velocity = static_cast<float>(packet.paramsObj.value("velocity").toDouble());
    cpParams.power = static_cast<float>(packet.paramsObj.value("power").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(false);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetCPLECmd(info.id, &cpParams, isQueued, &queuedCmdIndex);

    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(500, 5000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }

    return getCmdResultObj(packet, res);
}


/* ARC 功能 (1.14.1 ~ 1.15.2) */
//![1.14.1]
QJsonObject MagicDeviceController::pSetARCParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    ARCParams arcParams;
    quint64 queuedCmdIndex = 0;

    arcParams.xyzVelocity = static_cast<float>(packet.paramsObj.value("xyzVelocity").toDouble());
    arcParams.rVelocity = static_cast<float>(packet.paramsObj.value("rVelocity").toDouble());
    arcParams.xyzAcceleration = static_cast<float>(packet.paramsObj.value("xyzAcceleration").toDouble());
    arcParams.rAcceleration = static_cast<float>(packet.paramsObj.value("rAcceleration").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetARCParams(info.id, &arcParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.14.2]
QJsonObject MagicDeviceController::pGetARCParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    ARCParams arcParams;

    int res = GetARCParams(info.id, &arcParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("xyzVelocity", static_cast<double>(arcParams.xyzVelocity));
        paramsObj.insert("rVelocity", static_cast<double>(arcParams.rVelocity));
        paramsObj.insert("xyzAcceleration", static_cast<double>(arcParams.xyzAcceleration));
        paramsObj.insert("rAcceleration", static_cast<double>(arcParams.rAcceleration));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.14.3]
QJsonObject MagicDeviceController::pSetARCCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    QJsonObject cirPoint = packet.paramsObj.value("cirPoint").toObject();
    QJsonObject toPoint = packet.paramsObj.value("toPoint").toObject();

    ARCCmd arcCmd;
    arcCmd.cirPoint.x = static_cast<float>(cirPoint.value("x").toDouble());
    arcCmd.cirPoint.y = static_cast<float>(cirPoint.value("y").toDouble());
    arcCmd.cirPoint.z = static_cast<float>(cirPoint.value("z").toDouble());
    arcCmd.cirPoint.r = static_cast<float>(cirPoint.value("r").toDouble());
    arcCmd.toPoint.x = static_cast<float>(toPoint.value("x").toDouble());
    arcCmd.toPoint.y = static_cast<float>(toPoint.value("y").toDouble());
    arcCmd.toPoint.z = static_cast<float>(toPoint.value("z").toDouble());
    arcCmd.toPoint.r = static_cast<float>(toPoint.value("r").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(false);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetARCCmd(info.id, &arcCmd, isQueued, &queuedCmdIndex);

#if 1
    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(100, 5000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
#endif
    return getCmdResultObj(packet, res);
}

//![1.14.4]
QJsonObject MagicDeviceController::pSetCircleCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    quint64 queuedCmdIndex = 0;

    QJsonObject cirPoint = packet.paramsObj.value("cirPoint").toObject();
    QJsonObject toPoint = packet.paramsObj.value("toPoint").toObject();

    CircleCmd circleCmd;
    circleCmd.cirPoint.x = static_cast<float>(cirPoint.value("x").toDouble());
    circleCmd.cirPoint.y = static_cast<float>(cirPoint.value("y").toDouble());
    circleCmd.cirPoint.z = static_cast<float>(cirPoint.value("z").toDouble());
    circleCmd.cirPoint.r = static_cast<float>(cirPoint.value("r").toDouble());
    circleCmd.toPoint.x = static_cast<float>(toPoint.value("x").toDouble());
    circleCmd.toPoint.y = static_cast<float>(toPoint.value("y").toDouble());
    circleCmd.toPoint.z = static_cast<float>(toPoint.value("z").toDouble());
    circleCmd.toPoint.r = static_cast<float>(toPoint.value("r").toDouble());

    bool isWaitForFinish = packet.paramsObj.value("isWaitForFinish").toBool(false);
    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetCircleCmd(info.id, &circleCmd, isQueued, &queuedCmdIndex);

#if 1
    /* wait for finish */
    if (isWaitForFinish == true && res == DobotCommunicate_NoError && isQueued == true) {
        MWaitForFinish *wait = new MWaitForFinish(info.id, queuedCmdIndex, packet, this);
        wait->startWaiting(100, 5000);
        connect(wait, &MWaitForFinish::finish_signal, this, &MagicDeviceController::waitForFinish_slot);
        m_waitFinishMap.insert(wait, info.id);
        return QJsonObject();
    }
#endif
    return getCmdResultObj(packet, res);
}


/* Lost Step 丢步检测 (1.15.1 ~ 1.15.2) */
//![1.15.1]
QJsonObject MagicDeviceController::pSetLostStepParams(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    float threshold = static_cast<float>(packet.paramsObj.value("threshold").toDouble());

    int res = SetLostStepParams(info.id, threshold);
    return getCmdResultObj(packet, res);
}

//![1.15.2]
QJsonObject MagicDeviceController::pSetLostStepCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);
    quint64 queuedCmdIndex = 0;

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetLostStepCmd(info.id, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}


/* WAIT 功能 (1.16.1 ~ 1.16.2) */
//![1.16.1]
QJsonObject MagicDeviceController::pSetWAITCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    WAITCmd waitParams;
    quint64 queuedCmdIndex = 0;

    waitParams.timeout = static_cast<quint32>(packet.paramsObj.value("timeout").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetWAITCmd(info.id, &waitParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.16.2]
QJsonObject MagicDeviceController::pSetTRIGCmd(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    TRIGCmd trigParams;
    quint64 queuedCmdIndex = 0;

    trigParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());
    trigParams.mode = static_cast<quint8>(packet.paramsObj.value("mode").toInt());
    trigParams.condition = static_cast<quint8>(packet.paramsObj.value("condition").toInt());
    trigParams.threshold = static_cast<quint16>(packet.paramsObj.value("threshold").toInt());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetTRIGCmd(info.id, &trigParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}


/* EIO 功能(1.17.1 ~ 1.17.10) */
//![1.17.1]
QJsonObject MagicDeviceController::pSetIOMultiplexing(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    IOMultiplexing eioParams;
    quint64 queuedCmdIndex = 0;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());      //IO 地址，取值范围：1~20
    eioParams.multiplex = static_cast<quint8>(packet.paramsObj.value("multiplex").toInt());  //IO 功能。取值范围：0~6

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetIOMultiplexing(info.id, &eioParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.17.2]
QJsonObject MagicDeviceController::pGetIOMultiplexing(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    IOMultiplexing eioParams;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());

    int res = GetIOMultiplexing(info.id, &eioParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("address", static_cast<double>(eioParams.address));
        paramsObj.insert("multiplex", static_cast<double>(eioParams.multiplex));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.3]
QJsonObject MagicDeviceController::pSetIODO(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    IODO eioParams;
    quint64 queuedCmdIndex = 0;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());  //IO 地址
    eioParams.level = static_cast<quint8>(packet.paramsObj.value("level").toInt());      //输出电平 0：低 1：高

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetIODO(info.id, &eioParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.17.4]
QJsonObject MagicDeviceController::pGetIODO(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    IODO eioParams;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());

    int res = GetIODO(info.id, &eioParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("address", eioParams.address);
        paramsObj.insert("level", eioParams.level);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.5]
QJsonObject MagicDeviceController::pSetIOPWM(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    IOPWM eioParams;
    quint64 queuedCmdIndex = 0;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());      //IO 地址
    eioParams.frequency = static_cast<float>(packet.paramsObj.value("frequency").toDouble()); //PWM频率。取值范围：10HZ~1MHz
    eioParams.dutyCycle = static_cast<float>(packet.paramsObj.value("dutyCycle").toDouble()); //PWM占空比。取值范围：0~100

    bool isQueued = packet.paramsObj.value("isQueued").toBool(false);

    int res = SetIOPWM(info.id, &eioParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.17.6]
QJsonObject MagicDeviceController::pGetIOPWM(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    IOPWM eioParams;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());

    int res = GetIOPWM(info.id, &eioParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("address", eioParams.address);
        paramsObj.insert("frequency", static_cast<double>(eioParams.frequency));
        paramsObj.insert("dutyCycle", static_cast<double>(eioParams.dutyCycle));
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.7]
QJsonObject MagicDeviceController::pGetIODI(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    IODI eioParams;

    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());

    int res = GetIODI(info.id, &eioParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("address", eioParams.address);
        paramsObj.insert("level", eioParams.level);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.8]
QJsonObject MagicDeviceController::pGetIOADC(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    IOADC eioParams;
    eioParams.address = static_cast<quint8>(packet.paramsObj.value("address").toInt());

    int res = GetIOADC(info.id, &eioParams);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("address", eioParams.address);
        paramsObj.insert("value", eioParams.value);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.9]
QJsonObject MagicDeviceController::pSetEMotor(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    EMotor eioParams;
    quint64 queuedCmdIndex = 0;

    eioParams.index = static_cast<quint8>(packet.paramsObj.value("index").toInt());         //电机编号，0:Stepper1;2:Stepper2
    eioParams.isEnabled = static_cast<quint8>(packet.paramsObj.value("enable").toInt());   //电机控制使能。0：未使能。1：使能
    eioParams.speed = static_cast<qint32>(packet.paramsObj.value("speed").toDouble());      //电机控制速度（脉冲个数每秒）

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetEMotor(info.id, &eioParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.17.10]
QJsonObject MagicDeviceController::pSetEMotorS(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    EMotorS eioParams;
    quint64 queuedCmdIndex = 0;

    eioParams.index = static_cast<quint8>(packet.paramsObj.value("index").toInt());
    eioParams.isEnabled = static_cast<quint8>(packet.paramsObj.value("enable").toInt());
    eioParams.speed = static_cast<qint32>(packet.paramsObj.value("speed").toDouble());
    eioParams.distance = static_cast<quint32>(packet.paramsObj.value("distance").toDouble());

    bool isQueued = packet.paramsObj.value("isQueued").toBool(true);

    int res = SetEMotorS(info.id, &eioParams, isQueued, &queuedCmdIndex);
    return getCmdResultObj(packet, res);
}

//![1.17.11]
QJsonObject MagicDeviceController::pSetInfraredSensor(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    bool enable = packet.paramsObj.value("enable").toBool();
    int port = packet.paramsObj.value("infraredPort").toInt();
    int version = packet.paramsObj.value("version").toInt();

    InfraredPort infraredPort = IF_PORT_GP1;

    if (port >= 0 && port <= 5) {
        infraredPort = static_cast<InfraredPort>(port);
    } else {
        qDebug() << "InvalidParameterValue: infraredPort value is out of range.";
    }

    int res = SetInfraredSensor(info.id, enable, infraredPort, static_cast<quint8>(version));
    return getCmdResultObj(packet, res);
}

//![1.17.12]
QJsonObject MagicDeviceController::pGetInfraredSensor(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    quint8 value = 0;
    InfraredPort infraredPort = IF_PORT_GP1;

    int port = packet.paramsObj.value("infraredPort").toInt();

    if (port >= 0 && port <= 5) {
        infraredPort = static_cast<InfraredPort>(port);
    } else {
        qDebug() << "InvalidParameterValue: InfraredPort value is out of range.";
    }

    int res = GetInfraredSensor(info.id, infraredPort, &value);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert(QString("infraredPort%1").arg(port + 1), value);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

//![1.17.13]
QJsonObject MagicDeviceController::pSetColorSensor(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QString portName = packet.paramsObj.value("portName").toString();
    bool enable = packet.paramsObj.value("enable").toBool();
    int port = packet.paramsObj.value("colorPort").toInt();
    int version = packet.paramsObj.value("version").toInt();

    ColorPort colorPort = CL_PORT_GP1;

    if (port >= 0 && port <= 5) {
        colorPort = static_cast<ColorPort>(port);
    } else {
        qDebug() << "InvalidParameterValue: colorPort value is out of range.";
    }

    int res = SetColorSensor(info.id, enable, colorPort, static_cast<quint8>(version));
    return getCmdResultObj(packet, res);
}

//![1.17.14]
QJsonObject MagicDeviceController::pGetColorSensor(const MagicianPacket &packet)
{
    qDebug() << __FUNCTION__;
    DeviceInfo info = m_DevInfoMap.value(packet.portName);

    QJsonObject paramsObj;
    quint8 r, g, b;

    int res = GetColorSensor(info.id, &r, &g, &b);
    if (res == DobotCommunicate_NoError) {
        paramsObj.insert("red", r);
        paramsObj.insert("green", g);
        paramsObj.insert("blue", b);
    }
    return getCmdResultObj(packet, res, paramsObj);
}

/* SLOT */
void MagicDeviceController::handleCheckTimeout_slot()
{
    if (m_ConnectedDeviceCount > 0) {
        updataDeviceState();
    }
}

/* 退出前处理 */
void MagicDeviceController::closeAllDevice_slot()
{
    checkStateTimer->stop();

    foreach (const QString &portName, m_DevInfoMap.keys()) {
        pDisConnectDobot(portName);
    }

    delete checkStateTimer;
}

/* 等待运动结束的槽 */
void MagicDeviceController::waitForFinish_slot(int res)
{
    auto wait = qobject_cast<MWaitForFinish*>(sender());
    m_waitFinishMap.remove(wait);

    emit sendFinishPacket_signal(getCmdResultObj(wait->m_packet, res));

    wait->deleteLater();
}

int MagicDeviceController::_getConnecteDeviceCount()
{
    int count = 0;
    QMap<QString, DeviceInfo>::const_iterator i;
    for (i = m_DevInfoMap.constBegin(); i != m_DevInfoMap.constEnd(); ++i) {
        if (i.value().isConnected) {
           count++;
        }
    }
    return count;
}
