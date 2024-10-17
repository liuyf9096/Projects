#include "Device.h"

#include "Module.h"

#include <QTimer>
#include <QVariant>

const QString BasePort = "22000";
const QString DefaultIPAddress = "192.168.5.1";

Device::Device(QObject *parent) : QObject(parent)
{
    m_isConnected = false;

    _apiFunctionInit();
    _selfExchangeFunMapInit();

    m_modebug = new Mobdebug(this);
    connect(m_modebug, &Mobdebug::onModebugMessage_signal,
            this, &Device::onModebugMessage_slot);
    connect(m_modebug, &Mobdebug::onModebugStateChanged_signal,
            this, &Device::onModebugStateChanged_slot);
    connect(m_modebug, &Mobdebug::onFinish_signal,
            this, &Device::onModebugFinish_slot);
    connect(m_modebug, &Mobdebug::onErrorOccured_signal,
            this, &Device::onModebugErrorOccured_slot);

    m_module = new Module(BasePort, this);
    connect(m_module, &Module::onReceiveData_signal,
            this, &Device::onReplyMessage_slot);
}

void Device::sendCommand(QString api, quint64 id, QJsonObject params)
{
    APIFunction function = m_FuncMap.value(api);
    if (function) {
        (this->*function)(id, params);
        m_requestMap.insert(id, params);
    } else {
        qDebug() << "No matching function with: " << api << " params:" << params;
        emit onErrorOccured_signal(id, 111);
    }
}

void Device::setWsPort(quint16 port)
{
    m_wsPort = port;
}

void Device::_apiFunctionInit()
{
    /* 万能函数 */
    m_FuncMap.insert("SendPostCmd", &Device::pSendPostCmd);
    m_FuncMap.insert("SendGetCmd", &Device::pSendGetCmd);
    m_FuncMap.insert("ReadFile", &Device::pReadFile);
    m_FuncMap.insert("WriteFile", &Device::pWriteFile);

    /* 3. 连接状态 */
    m_FuncMap.insert("SearchDobot", &Device::pSearchDobot);
    m_FuncMap.insert("ConnectDobot", &Device::pConnectDobot);
    m_FuncMap.insert("DisconnectDobot", &Device::pDisconnectDobot);
    m_FuncMap.insert("GetConnectionState", &Device::pGetConnectionState);

    /* 4. 基础数据交互 */
    m_FuncMap.insert("Exchange", &Device::pExchange);

    m_FuncMap.insert("GetDobotStatus", &Device::pGetDobotStatus);

    m_FuncMap.insert("SetControlMode", &Device::pSetControlMode);
    m_FuncMap.insert("GetControlMode", &Device::pGetControlMode);

    m_FuncMap.insert("SetCRControlMode", &Device::pSetCRControlMode);
    m_FuncMap.insert("SetIRControlMode", &Device::pSetIRControlMode);

    m_FuncMap.insert("SetCoordinateMode", &Device::pSetCoordinateMode);
    m_FuncMap.insert("GetCoordinateMode", &Device::pGetCoordinateMode);

    m_FuncMap.insert("SetJogMode", &Device::pSetJogMode);
    m_FuncMap.insert("GetJogMode", &Device::pGetJogMode);

    m_FuncMap.insert("SetToolCoordPattern", &Device::pSetToolCoordPattern);
    m_FuncMap.insert("GetToolCoordPattern", &Device::pGetToolCoordPattern);

    m_FuncMap.insert("SetUserCoordPattern", &Device::pSetUserCoordPattern);
    m_FuncMap.insert("GetUserCoordPattern", &Device::pGetUserCoordPattern);

    m_FuncMap.insert("GetAutoManualMode", &Device::pGetAutoManualMode);

    m_FuncMap.insert("GetJointCoordinate", &Device::pGetJointCoordinate);
    m_FuncMap.insert("GetCartesianCoordinate", &Device::pGetCartesianCoordinate);
    m_FuncMap.insert("GetAlarms", &Device::pGetAlarms);
    m_FuncMap.insert("ClearAlarms", &Device::pClearAlarms);
    m_FuncMap.insert("GetInput", &Device::pGetInput);
    m_FuncMap.insert("SetOutput", &Device::pSetOutput);
    m_FuncMap.insert("GetOutput", &Device::pGetOutput);

    /* 5. 参数设置 */
    /* 5.2 全局比例 */
    m_FuncMap.insert("SetCommonSetting", &Device::pSetCommonSetting);
    m_FuncMap.insert("GetCommonSetting", &Device::pGetCommonSetting);

    /* 5.3 示教参数 */
    m_FuncMap.insert("SetTeachJoint", &Device::pSetTeachJoint);
    m_FuncMap.insert("GetTeachJoint", &Device::pGetTeachJoint);

    m_FuncMap.insert("SetTeachCoordinate", &Device::pSetTeachCoordinate);
    m_FuncMap.insert("GetTeachCoordinate", &Device::pGetTeachCoordinate);

    /* 5.4 再现参数 */
    m_FuncMap.insert("SetPlaybackJoint", &Device::pSetPlaybackJoint);
    m_FuncMap.insert("GetPlaybackJoint", &Device::pGetPlaybackJoint);

    m_FuncMap.insert("SetPlaybackCoordinate", &Device::pSetPlaybackCoordinate);
    m_FuncMap.insert("GetPlaybackCoordinate", &Device::pGetPlaybackCoordinate);

    m_FuncMap.insert("SetPlaybackArch", &Device::pSetPlaybackArch);
    m_FuncMap.insert("GetPlaybackArch", &Device::pGetPlaybackArch);

    /* 5.5~5.6 坐标系 */
    m_FuncMap.insert("SetToolCoordinate", &Device::pSetToolCoordinate);
    m_FuncMap.insert("GetToolCoordinate", &Device::pGetToolCoordinate);
    m_FuncMap.insert("SetUserCoordinate", &Device::pSetUserCoordinate);
    m_FuncMap.insert("GetUserCoordinate", &Device::pGetUserCoordinate);

    /* 5.7 示教点列表 */
    m_FuncMap.insert("SetTeachFileUpdate", &Device::pSetTeachFileUpdate);
    m_FuncMap.insert("DelTeachFileDelete", &Device::pDelTeachFileDelete);

    /* 6.1 工具坐标系标定 */
    m_FuncMap.insert("SetCalibrateToolCoor", &Device::pSetCalibrateToolCoor);
    m_FuncMap.insert("SetCalibrateToolPosition", &Device::pSetCalibrateToolPosition);
    m_FuncMap.insert("SetCalibrateToolPose", &Device::pSetCalibrateToolPose);

    /* 6.2 用户坐标系标定 */
    m_FuncMap.insert("SetCalibrateUserCoor", &Device::pSetCalibrateUserCoor);

    /* 6.3 左右手标定 */
    m_FuncMap.insert("SetCalibrateLeftRightHand", &Device::pSetCalibrateLeftRightHand);

    /* 6.4 回零 */
    m_FuncMap.insert("SetCalibrateHome", &Device::pSetCalibrateHome);

    /* 7.1~7.10 调试器 */
    m_FuncMap.insert("GetDebuggerState", &Device::pGetDebuggerState);
    m_FuncMap.insert("SetDebuggerStart", &Device::pSetDebuggerStart);
    m_FuncMap.insert("SetDebuggerStop", &Device::pSetDebuggerStop);
    m_FuncMap.insert("SetDebuggerRun", &Device::pSetDebuggerRun);
    m_FuncMap.insert("SetDebuggerSuspend", &Device::pSetDebuggerSuspend);
    m_FuncMap.insert("SetDebuggerStepIn", &Device::pSetDebuggerStepIn);
    m_FuncMap.insert("SetDebuggerStepOver", &Device::pSetDebuggerStepOver);
    m_FuncMap.insert("SetDebuggerSetb", &Device::pSetDebuggerSetb);
    m_FuncMap.insert("SetDebuggerDelb", &Device::pSetDebuggerDelb);
    m_FuncMap.insert("SetDebuggerDelAllb", &Device::pSetDebuggerDelAllb);

    /* 8. 面板模拟 */
    m_FuncMap.insert("SetThreeSwitch", &Device::pSetThreeSwitch);
    m_FuncMap.insert("SetAutoManualMode", &Device::pSetAutoManualMode);
    m_FuncMap.insert("SetEmergencyStop", &Device::pSetEmergencyStop);
    m_FuncMap.insert("SetJogCmd", &Device::pSetJogCmd);

    /* 9. 网络设置 */
    m_FuncMap.insert("SetEthernet", &Device::pSetEthernet);
    m_FuncMap.insert("GetEthernet", &Device::pGetEthernet);

    /* 10.1 获取相机坐标 */
    m_FuncMap.insert("SetVisionCoordinate", &Device::pSetVisionCoordinate);

    /* 10.2 运动到指定点 */
    m_FuncMap.insert("SetMoveCmd", &Device::pSetMoveCmd);

    /* 11. 传送带跟踪 */
    m_FuncMap.insert("GetCurrentEncoder", &Device::pGetCurrentEncoder);
    m_FuncMap.insert("GetSensorEncoder", &Device::pGetSensorEncoder);
    m_FuncMap.insert("SetSensorEncoderListen", &Device::pSetSensorEncoderListen);

    /* 12. 手动模式中全局速度比例 */
    m_FuncMap.insert("SetManualSpeedRatio", &Device::pSetManualSpeedRatio);
    m_FuncMap.insert("GetManualSpeedRatio", &Device::pGetManualSpeedRatio);

    /* 13. DI 模拟输入 */
    m_FuncMap.insert("SetDIMode", &Device::pSetDIMode);
    m_FuncMap.insert("GetDIMode", &Device::pGetDIMode);
    m_FuncMap.insert("SetDIValue", &Device::pSetDIValue);

    /* 14. 通用IO模拟量 */
    m_FuncMap.insert("SetGPIOAO", &Device::pSetGPIOAO);
    m_FuncMap.insert("GetGPIOAO", &Device::pGetGPIOAO);
    m_FuncMap.insert("SetGPIOAI", &Device::pSetGPIOAI);
    m_FuncMap.insert("GetGPIOAI", &Device::pGetGPIOAI);

    /* 15. 协作机器人安全配置 */
    m_FuncMap.insert("SetGeneralSafeSetting", &Device::pSetGeneralSafeSetting);
    m_FuncMap.insert("SetLoadParams", &Device::pSetLoadParams);
    m_FuncMap.insert("SetAdvancedFunc", &Device::pSetAdvancedFunc);
    m_FuncMap.insert("SetCollisionDetect", &Device::pSetCollisionDetect);
    m_FuncMap.insert("SetSafeParams", &Device::pSetSafeParams);

    /* 16. 电子皮肤 */
    m_FuncMap.insert("SetElecSkinEnable", &Device::pSetElecSkinEnable);
    m_FuncMap.insert("SetElecSkinReset", &Device::pSetElecSkinReset);
    m_FuncMap.insert("GetElecSkinReset", &Device::pGetElecSkinReset);
    m_FuncMap.insert("SetElecSkinParams", &Device::pSetElecSkinParams);
    m_FuncMap.insert("GetElecSkinParams", &Device::pGetElecSkinParams);

    /* 17. 回零 */
    m_FuncMap.insert("SetGoHomeCmd", &Device::pSetGoHomeCmd);

    /* 18. 开关抱闸 */
    m_FuncMap.insert("SetAxisJointBrake", &Device::pSetAxisJointBrake);
    m_FuncMap.insert("GetAxisJointBrake", &Device::pGetAxisJointBrake);

    /* 19. 轨迹复现 */
    m_FuncMap.insert("SetRecurrentTrack", &Device::pSetRecurrentTrack);
    m_FuncMap.insert("SetDebugReTrace", &Device::pSetDebugReTrace);
    m_FuncMap.insert("GetDebugReTrace", &Device::pGetDebugReTrace);

    /* 20. 协作机器人自动识别 */
    m_FuncMap.insert("SetAutoIdentify", &Device::pSetAutoIdentify);

    /* 21.协作机器人安装位置 */
    m_FuncMap.insert("SetInstallPosture", &Device::pSetInstallPosture);

    /* 22.协作机器人实轴、虚轴 */
    m_FuncMap.insert("SetSimulatedAxies", &Device::pSetSimulatedAxies);

    /* 23.协作机器人末端执行器 */
    m_FuncMap.insert("SetRobottiqGripperEnable", &Device::pSetRobottiqGripperEnable);
    m_FuncMap.insert("GetRobottiqGripperEnable", &Device::pGetRobottiqGripperEnable);
    m_FuncMap.insert("SetRobottiqGripperEnableK", &Device::pSetRobottiqGripperEnableK);
    m_FuncMap.insert("SetHitbotGripperEnable", &Device::pSetHitbotGripperEnable);
    m_FuncMap.insert("GetHitbotGripperEnable", &Device::pGetHitbotGripperEnable);
    m_FuncMap.insert("SetHitbotGripper", &Device::pSetHitbotGripper);
    m_FuncMap.insert("SetDHGripperEnable", &Device::pSetDHGripperEnable);
    m_FuncMap.insert("GetDHGripperEnable", &Device::pGetDHGripperEnable);
    m_FuncMap.insert("SetDHGripper", &Device::pSetDHGripper);
    m_FuncMap.insert("GetDHGripper", &Device::pGetDHGripper);

    /* 24.协作机器人 6维力传感器接口 */
    m_FuncMap.insert("SetRobotiqSixForce", &Device::pSetRobotiqSixForce);
    m_FuncMap.insert("SetRobotiqSixForce", &Device::pSetRobotiqSixForce);
    m_FuncMap.insert("GetRobotiqSixForce", &Device::pGetRobotiqSixForce);

    /* 24.2 模拟量接口 */
    m_FuncMap.insert("SetEndAI", &Device::pSetEndAI);
    m_FuncMap.insert("GetEndAI", &Device::pGetEndAI);

    /* 25.协作控制器上下电 */
    m_FuncMap.insert("SetPowerControl", &Device::pSetPowerControl);
    m_FuncMap.insert("GetPowerControl", &Device::pGetPowerControl);

    /* 27.协作机器人RUNTO */
    m_FuncMap.insert("SetCRRunTo", &Device::pSetCRRunTo);

    /* 28. 版本号 */
    m_FuncMap.insert("GetVersion", &Device::pGetVersion);
}

void Device::_selfExchangeFunMapInit()
{
    m_selfExchangeFunMap.insert("SetControlMode", "controlMode");
    m_selfExchangeFunMap.insert("GetControlMode", "controlMode");
    m_selfExchangeFunMap.insert("SetCoordinateMode", "coordinate");
    m_selfExchangeFunMap.insert("GetCoordinateMode", "coordinate");
    m_selfExchangeFunMap.insert("SetJogMode", "jogMode");
    m_selfExchangeFunMap.insert("GetJogMode", "jogMode");
    m_selfExchangeFunMap.insert("SetToolCoordPattern", "toolCoordinate");
    m_selfExchangeFunMap.insert("GetToolCoordPattern", "toolCoordinate");
    m_selfExchangeFunMap.insert("SetUserCoordPattern", "userCoordinate");
    m_selfExchangeFunMap.insert("GetUserCoordPattern", "userCoordinate");
    m_selfExchangeFunMap.insert("GetAutoManualMode", "autoManual");
    m_selfExchangeFunMap.insert("GetJointCoordinate", "jointCoordinate");
    m_selfExchangeFunMap.insert("GetCartesianCoordinate", "cartesianCoordinate");
    m_selfExchangeFunMap.insert("ClearAlarms", "alarms");
    m_selfExchangeFunMap.insert("GetAlarms", "alarms");
    m_selfExchangeFunMap.insert("GetInput", "inputs");
    m_selfExchangeFunMap.insert("SetOutput", "outputs");
    m_selfExchangeFunMap.insert("GetOutput", "outputs");
}

void Device::pSearchDobot(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->setIpAddress("192.168.5.1");
    m_module->sendGetRequest("/connection/state", id, "SearchDobot");
}

void Device::pConnectDobot(quint64 id, QJsonObject params)
{
    checkStringValue(params, "portName");

    QString portName = params.value("portName").toString();

    if (!portName.isEmpty()) {
        m_module->setIpAddress(portName);
    } else {
        m_module->setIpAddress(DefaultIPAddress);
    }

    m_module->sendGetRequest("/connection/state", id, "ConnectDobot");
}

void Device::pDisconnectDobot(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_isConnected = false;
    m_modebug->udpClose();

    emit onReplyMessage_signal(id, true);
}

//![万能函数]
void Device::pSendPostCmd(quint64 id, QJsonObject params)
{
    checkStringValue(params, "url");
    checkObjectValue(params, "body");

    QString portName = params.value("portName").toString();
    QString url = params.value("url").toString();
    QJsonObject body = params.value("body").toObject();

    m_module->sendPostRequest(url, body, id);
}

void Device::pSendGetCmd(quint64 id, QJsonObject params)
{
    checkStringValue(params, "url");

    QString url = params.value("url").toString();
    m_module->sendGetRequest(url, id);
}

void Device::pReadFile(quint64 id, QJsonObject params)
{
    checkStringValue(params, "fileName");

    QString fileName = params.value("fileName").toString();
    QByteArray content;
    bool ok = m_module->readFile(fileName, content);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(content);
        emit onReplyMessage_signal(id, value);
    } else {
        qDebug() << "read file failed.";
        emit onErrorOccured_signal(id, 666);
    }
}

void Device::pWriteFile(quint64 id, QJsonObject params)
{
    checkStringValue(params, "url");
    checkStringValue(params, "fileName");
    checkObjectValue(params, "content");

    QString url = params.value("url").toString();
    QString fileName = params.value("fileName").toString();
    QJsonValue contentValue = params.value("content");
    if (contentValue.isObject()) {
        QJsonObject contentObj = contentValue.toObject();
        bool ok = m_module->writeFile(fileName, contentObj);
        if (ok and !url.isEmpty()) {
            m_module->sendPostRequest(url, contentObj, id);
        } else {
            qDebug() << "write file failed.";
            emit onErrorOccured_signal(id, 666);
        }
    } else if (contentValue.isString()) {
        bool ok = m_module->writeFile(fileName, contentValue.toString());
        if (ok) {
            emit onReplyMessage_signal(id, true);
        } else {
            qDebug() << "write file failed.";
            emit onErrorOccured_signal(id, 666);
        }
    }
}

//![3.1] 连接状态
void Device::pGetConnectionState(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/connection/state", id, "GetConnectionState");
}

//![4.1] 周期性命令和数据交互
void Device::pExchange(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    if (!dataObj.isEmpty()) {
        m_module->sendPostRequest("/protocol/exchange", dataObj, id, "Exchange");
    } else {
        qDebug() << "data obj is missing.";
        emit onErrorOccured_signal(id, 666);
    }
}

void Device::pGetDobotStatus(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetDobotStatus");
}

void Device::pSetControlMode(quint64 id, QJsonObject params)
{
    if (checkStringValue(params, "mode")) {
        RobotStatus status(m_status);
        status.setControlMode(params.value("mode").toString());

        _sendStatus(id, status, "SetControlMode");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetControlMode(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetControlMode");
}

void Device::pSetCoordinateMode(quint64 id, QJsonObject params)
{ 
    if (checkStringValue(params, "mode")) {
        RobotStatus status(m_status);
        status.setCoordinateMode(params.value("mode").toString());

        _sendStatus(id, status, "SetCoordinateMode");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetCoordinateMode(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetCoordinateMode");
}

void Device::pSetJogMode(quint64 id, QJsonObject params)
{
    if (checkStringValue(params, "mode")) {
        RobotStatus status(m_status);
        status.setJogMode(params.value("mode").toString());

        _sendStatus(id, status, "SetJogMode");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetJogMode(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetJogMode");
}

void Device::pSetToolCoordPattern(quint64 id, QJsonObject params)
{
    if (checkIntValue(params, "pattern")) {
        RobotStatus status(m_status);
        status.setToolCoordPattern(params.value("pattern").toInt());

        _sendStatus(id, status, "SetToolCoordPattern");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetToolCoordPattern(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetToolCoordPattern");
}

void Device::pSetUserCoordPattern(quint64 id, QJsonObject params)
{
    if (checkIntValue(params, "pattern")) {
        RobotStatus status(m_status);
        status.setUserCoordPattern(params.value("pattern").toInt());

        _sendStatus(id, status, "SetUserCoordPattern");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetUserCoordPattern(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetUserCoordPattern");
}

void Device::pGetAutoManualMode(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetAutoManualMode");
}

void Device::pGetJointCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetJointCoordinate");
}

void Device::pGetCartesianCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetCartesianCoordinate");
}

void Device::pGetAlarms(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetAlarms");
}

void Device::pClearAlarms(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    RobotStatus status(m_status);
    status.clearAlarms();

    _sendStatus(id, status, "ClearAlarms");
}

void Device::pGetInput(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetInput");
}

void Device::pSetOutput(quint64 id, QJsonObject params)
{

    if (checkBoolValue(params, "enable")) {
        RobotStatus status(m_status);

        bool enable = params.value("enable").toBool();

        checkArrayValue(params, "outputs");
        QJsonArray outputs = params.value("outputs").toArray();
        status.setOutputs(enable, outputs);

        _sendStatus(id, status, "SetOutput");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetOutput(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    _getStatus(id, "GetOutput");
}

/* 5.参数设定 */
//![5.2] 全局比例
void Device::pSetCommonSetting(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    bool ok = m_module->writeFile("/project/settings/common.json", dataObj);
    if (ok) {
        m_module->sendPostRequest("/settings/common", dataObj, id, "SetCommonSetting");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetCommonSetting(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/common.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

/* 5.3 示教参数 */
//![5.3.1] 示教关节点动
void Device::pSetTeachJoint(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    bool ok = m_module->writeFile("/project/settings/teach/joint.json", dataObj);
    if (ok) {
        m_module->sendPostRequest("/settings/teach/joint", dataObj, id, "SetTeachJoint");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetTeachJoint(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/teach/joint.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

//![5.3.2] 示教坐标系点动
void Device::pSetTeachCoordinate(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    bool ok = m_module->writeFile("/project/settings/teach/coordinate.json", dataObj);
    if (ok) {
        m_module->sendPostRequest("/settings/teach/coordinate", dataObj, id, "SetTeachCoordinate");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetTeachCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/teach/coordinate.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

/* 5.4 再现参数*/
//![5.4.1] 关节空间规划参数
void Device::pSetPlaybackJoint(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    bool ok = m_module->writeFile("/project/settings/playback/joint.json", dataObj);
    if (ok) {
        m_module->sendPostRequest("/settings/playback/joint", dataObj, id, "SetPlaybackJoint");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetPlaybackJoint(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/playback/joint.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

//![5.4.2] 笛卡尔空间规划参数
void Device::pSetPlaybackCoordinate(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    bool ok = m_module->writeFile("/project/settings/playback/coordinate.json", dataObj);
    if (ok) {
        m_module->sendPostRequest("/settings/playback/coordinate", dataObj, id, "SetPlaybackCoordinate");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetPlaybackCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/playback/coordinate.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

//![5.4.3] Arch参数
void Device::pSetPlaybackArch(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    bool ok = m_module->writeFile("/project/settings/playback/arch.json", dataValue);
    if (ok) {
        m_module->sendPostRequest("/settings/playback/arch", dataValue, id, "SetPlaybackArch");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetPlaybackArch(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/playback/arch.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

//![5.5] 工具坐标系
void Device::pSetToolCoordinate(quint64 id, QJsonObject params)
{
    QJsonValue data = params.value("data");

    bool ok = m_module->writeFile("/project/settings/coordinate/tool.json", data);
    if (ok) {
        m_module->sendPostRequest("/settings/coordinate/tool", data, id, "SetToolCoordinate");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetToolCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/coordinate/tool.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

//![5.6] 用户坐标系参数
void Device::pSetUserCoordinate(quint64 id, QJsonObject params)
{
    QJsonValue data = params.value("data");

    bool ok = m_module->writeFile("/project/settings/coordinate/user.json", data);
    if (ok) {
        m_module->sendPostRequest("/settings/coordinate/user", data, id, "SetUserCoordinate");
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

void Device::pGetUserCoordinate(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    QByteArray data;
    bool ok = m_module->readFile("/project/settings/coordinate/user.json", data);
    if (ok) {
        QJsonValue value = m_module->parseJsonData(data);
        emit onReplyMessage_signal(id, value);
    } else {
        emit onErrorOccured_signal(id, 6667);
    }
}

/* 5.7 示教点列表 */
//![5.7.1] 增加示教点文件
void Device::pSetTeachFileUpdate(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "file");

    m_module->sendPostRequest("/project/teachFileUpdate", dataObj, id, "SetTeachFileUpdate");
}

//![5.7.2] 删除示教点文件
void Device::pDelTeachFileDelete(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "file");

    m_module->sendPostRequest("/project/teachFileDelete", dataObj, id, "DelTeachFileDelete");
}

/* 6. 标定 */
//![6.1.1] 四轴工具坐标系标定
void Device::pSetCalibrateToolCoor(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/calibrate/coordinate/tool", dataValue, id, "SetCalibrateToolCoor");
}

//![6.1.2] 六轴工具坐标系位置标定
void Device::pSetCalibrateToolPosition(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/calibrate/coordinate/tool/position", dataValue, id, "SetCalibrateToolPosition");
}

//![6.1.3] 六轴工具坐标系姿态标定
void Device::pSetCalibrateToolPose(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/calibrate/coordinate/tool/pose", dataValue, id, "SetCalibrateToolPose");
}

//![6.2.1] 用户坐标系标定
void Device::pSetCalibrateUserCoor(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/calibrate/coordinate/user", dataValue, id, "SetCalibrateUserCoor");
}

//![6.3] 左右手标定
void Device::pSetCalibrateLeftRightHand(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkFloatValue(dataObj, "left");
    checkFloatValue(dataObj, "right");

    m_module->sendPostRequest("/calibrate/leftRightHand", dataObj, id, "SetCalibrateLeftRightHand");
}

//![6.4] 回零
void Device::pSetCalibrateHome(quint64 id, QJsonObject params)
{
    m_module->sendPostRequest("/calibrate/home", params.value("data"), id, "SetCalibrateHome");
}

/* 7. 调试器 */
//![7.1] 调试器状态
void Device::pGetDebuggerState(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/debugger/state", id, "GetDebuggerState");
}

//![7.2] 启动调试器（start)
void Device::pSetDebuggerStart(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_modebug->listen(id);
}

//![7.3] 停止调试器(stop)
void Device::pSetDebuggerStop(quint64 id, QJsonObject params)
{
    m_modebug->mo_exit();
    m_module->sendPostRequest("/debugger/stop", params, id, "SetDebuggerStop");
}

//![7.4] 运行(run)
void Device::pSetDebuggerRun(quint64 id, QJsonObject params)
{
    m_modebug->mo_run();
    m_module->sendPostRequest("/debugger/run", params, id, "SetDebuggerRun");
}

//![7.5] 暂停(suspend)
void Device::pSetDebuggerSuspend(quint64 id, QJsonObject params)
{
    m_modebug->mo_suspend();
    m_module->sendPostRequest("/debugger/suspend", params, id, "SetDebuggerSuspend");
}

//![7.6] 单步进入函数(stepIn)
void Device::pSetDebuggerStepIn(quint64 id, QJsonObject params)
{
    m_modebug->mo_step();
    m_module->sendPostRequest("/debugger/stepIn", params, id, "SetDebuggerStepIn");
}

//![7.7] 单步跳过函数(stepOver)
void Device::pSetDebuggerStepOver(quint64 id, QJsonObject params)
{
    m_modebug->mo_over();
    m_module->sendPostRequest("/debugger/stepOver", params, id, "SetDebuggerStepOver");
}

//![7.8] 设置断点(setb)
void Device::pSetDebuggerSetb(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    if (dataObj.contains("path") and dataObj.contains("line")) {
        m_module->sendPostRequest("/debugger/setb", dataObj, id, "SetDebuggerSetb");

        QString path = dataObj.value("path").toString();
        int line = dataObj.value("line").toInt();
        m_modebug->mo_setb(path, line);
    } else {
        qDebug() << "parameter missing.";
        emit onErrorOccured_signal(id, 6667);
    }
}

//![7.9] 移除断点(delb)
void Device::pSetDebuggerDelb(quint64 id, QJsonObject params)
{
    if (params.contains("path") and params.contains("line")) {
        m_module->sendPostRequest("/debugger/delb", params, id, "SetDebuggerDelb");

        QString path = params.value("path").toString();
        int line = params.value("line").toInt();
        m_modebug->mo_delb(path, line);
    } else {
        qDebug() << "parameter missing.";
        emit onErrorOccured_signal(id, 6667);
    }
}

//![7.10] 移除所有断点(delallb)
void Device::pSetDebuggerDelAllb(quint64 id, QJsonObject params)
{
    m_module->sendPostRequest("/debugger/delallb", params, id, "SetDebuggerDelAllb");
    m_modebug->mo_delallb();
}

/* 8. 面板模拟 */
//![8.1] 自动/手动
void Device::pSetAutoManualMode(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "value");

    m_module->sendPostRequest("/panel/autoManual", dataObj, id, "SetAutoManualMode");
}

//![8.2] 三位开关
void Device::pSetThreeSwitch(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "value");

    m_module->sendPostRequest("/panel/threeSwitch", dataObj, id, "SetThreeSwitch");
}

//![8.3] 急停状态
void Device::pSetEmergencyStop(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkBoolValue(dataObj, "value");

    m_module->sendPostRequest("/panel/emergencyStop", dataObj, id, "SetEmergencyStop");
}

//![8.4] 点动按键
void Device::pSetJogCmd(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkArrayValue(dataObj, "posBtns");
    checkArrayValue(dataObj, "negBtns");

    m_module->sendPostRequest("/panel/jog", dataObj, id, "SetJogCmd");
}

/* 9. 网络设置 */
//![9.1] 设置eth2
void Device::pSetEthernet(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkBoolValue(dataObj, "dhcp");
    checkStringValue(dataObj, "ip");
    checkStringValue(dataObj, "netmask");
    checkStringValue(dataObj, "gateway");

    m_module->sendPostRequest("/interface/ethernet", dataObj, id, "SetEthernet");
}

void Device::pGetEthernet(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/ethernet", id, "GetEthernet");
}

/* 10.1 获取相机坐标 */
void Device::pSetVisionCoordinate(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "ip");
    checkIntValue(dataObj, "port");
    checkIntValue(dataObj, "index");

    m_module->sendPostRequest("/interface/visionCoordinate", dataObj, id, "SetVisionCoordinate");
}

/* 10.2 运动到指定点 */
void Device::pSetMoveCmd(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkFloatValue(dataObj, "x");
    checkFloatValue(dataObj, "y");
    checkFloatValue(dataObj, "z");
    checkFloatValue(dataObj, "r");

    m_module->sendPostRequest("/interface/move", dataObj, id, "SetMoveCmd");
}

/* 11. 传送带跟踪 */
void Device::pGetCurrentEncoder(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "conveyor_index");

    m_module->sendPostRequest("/interface/readCurrentEncoder", dataObj, id, "GetCurrentEncoder");
}

void Device::pGetSensorEncoder(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "conveyor_index");

    m_module->sendPostRequest("/interface/readSensorEncoder", dataObj, id, "GetSensorEncoder");
}

void Device::pSetSensorEncoderListen(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendPostRequest("/interface/listenSensor", QJsonValue(), id, "SetSensorEncoderListen");
}

/* 12. 手动模式中全局速度比例 */
void Device::pSetManualSpeedRatio(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "ratio");

    m_module->sendPostRequest("/interface/manualCommon", dataObj, id, "SetManualSpeedRatio");
}

void Device::pGetManualSpeedRatio(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/manualCommon", id, "GetManualSpeedRatio");
}

/* 13. DI 模拟输入 */
void Device::pSetDIMode(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "index");
    checkIntValue(dataObj, "mode");

    m_module->sendPostRequest("/interface/setDIMode", dataObj, id, "SetDIMode");
}

void Device::pGetDIMode(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "index");

    m_module->sendPostRequest("/interface/getDIMode", dataObj, id, "GetDIMode");
}

void Device::pSetDIValue(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkIntValue(dataObj, "index");
    checkIntValue(dataObj, "value");

    m_module->sendPostRequest("/interface/setDIValue", dataObj, id, "SetDIValue");
}

/* 14. 通用IO模拟量 */
void Device::pSetGPIOAO(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/settings/function/gpioAO", dataValue, id, "SetGPIOAO");
}

void Device::pGetGPIOAO(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/settings/function/gpioAO", id, "GetGPIOAO");
}

void Device::pSetGPIOAI(quint64 id, QJsonObject params)
{
    QJsonValue dataValue = params.value("data");

    m_module->sendPostRequest("/settings/function/gpioAI", dataValue, id, "SetDIValue");
}

void Device::pGetGPIOAI(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/settings/function/gpioAI", id, "GetGPIOAO");
}

/* 15. 协作机器人安全配置 */
void Device::pSetGeneralSafeSetting(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/generalSafeSetting", dataObj, id, "SetGeneralSafeSetting");
}

void Device::pSetLoadParams(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/loadParams", dataObj, id, "SetLoadParams");
}

void Device::pSetAdvancedFunc(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/advancedFunction", dataObj, id, "SetAdvancedFunc");
}

void Device::pSetCollisionDetect(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkBoolValue(dataObj, "value");

    m_module->sendPostRequest("/settings/collisionDect", dataObj, id, "SetCollisionDetect");
}

void Device::pSetSafeParams(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/safeparams", dataObj, id, "SetSafeParams");
}

/* 16. 电子皮肤 */
void Device::pSetElecSkinEnable(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkBoolValue(dataObj, "value");

    m_module->sendPostRequest("/settings/function/elecSkin", dataObj, id, "SetElecSkinEnable");
}

void Device::pSetElecSkinReset(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/resetElecSkin", dataObj, id, "SetElecSkinReset");
}

void Device::pGetElecSkinReset(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/resetElecSkin", id, "GetElecSkinReset");
}

void Device::pSetElecSkinParams(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/elecSkinParams", dataObj, id, "SetElecSkinParams");
}

void Device::pGetElecSkinParams(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/settings/function/elecSkinParams", id, "GetElecSkinParams");
}

/* 17. 回零 */
void Device::pSetGoHomeCmd(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/goHome", dataObj, id, "SetGoHomeCmd");
}

/* 18. 开关抱闸 */
void Device::pSetAxisJointBrake(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/6axisJointBrake", dataObj, id, "SetAxisJointBrake");
}

void Device::pGetAxisJointBrake(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/6axisJointBrake", id, "GetAxisJointBrake");
}

/* 19. 轨迹复现 */
void Device::pSetRecurrentTrack(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/recurrentTrack", dataObj, id, "SetRecurrentTrack");
}

void Device::pSetDebugReTrace(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/debugReTrace", dataObj, id, "SetDebugReTrace");
}

void Device::pGetDebugReTrace(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/debugReTrace", id, "GetDebugReTrace");
}

/* 20. 协作机器人自动识别 */
void Device::pSetAutoIdentify(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/setting/autoIdentify", dataObj, id, "SetAutoIdentify");
}

/* 21.协作机器人安装位置 */
void Device::pSetInstallPosture(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/setting/installPosture", dataObj, id, "SetInstallPosture");
}

/* 22.协作机器人实轴、虚轴 */
void Device::pSetSimulatedAxies(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/setting/setSimulatedAxies", dataObj, id, "SetSimulatedAxies");
}

/* 23.协作机器人末端执行器 */
void Device::pSetRobottiqGripperEnable(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/setting/function/robotiqGripperEnable", dataObj, id, "SetRobottiqGripperEnable");
}

void Device::pGetRobottiqGripperEnable(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/setting/function/robotiqGripperEnable", id, "GetRobottiqGripperEnable");
}

void Device::pSetRobottiqGripperEnableK(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/robotiqGripperEnable", dataObj, id, "SetRobottiqGripperEnableK");
}

void Device::pSetHitbotGripperEnable(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/hitbotGripper", dataObj, id, "SetHitbotGripperEnable");
}

void Device::pGetHitbotGripperEnable(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/hitbotGripper", id, "GetHitbotGripperEnable");
}

void Device::pSetHitbotGripper(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/DHGripper", dataObj, id, "SetHitbotGripper");
}

void Device::pSetDHGripperEnable(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/DHGripperEnable", dataObj, id, "SetDHGripperEnable");
}

void Device::pGetDHGripperEnable(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/DHGripperEnable", id, "GetDHGripperEnable");
}

void Device::pSetDHGripper(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/DHGripper", dataObj, id, "SetDHGripper");
}

void Device::pGetDHGripper(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/DHGripper", id, "GetDHGripper");
}

/* 24.协作机器人 6维力传感器接口 */
void Device::pSetRobotiqSixForce(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/robotiqSixForce", dataObj, id, "SetRobotiqSixForce");
}

void Device::pGetRobotiqSixForce(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/settings/function/robotiqSixForce", id, "GetRobotiqSixForce");
}

/* 24.2 模拟量接口 */
void Device::pSetEndAI(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/settings/function/endAI", dataObj, id, "SetEndAI");
}

void Device::pGetEndAI(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/settings/function/endAI", id, "GetEndAI");
}

/* 25. */
void Device::pSetPowerControl(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/powerControl", dataObj, id, "SetPowerControl");
}

void Device::pGetPowerControl(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendGetRequest("/interface/powerControl", id, "GetPowerControl");
}

/* 26. 协作控制器上下电 */
void Device::pSetCRControlMode(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "controlMode");

    m_module->sendPostRequest("/settings/controlMode", dataObj, id, "SetCRControlMode");
}

/* 27.协作机器人RUNTO */
void Device::pSetCRRunTo(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    m_module->sendPostRequest("/interface/go", dataObj, id, "SetCRRunTo");
}

/* 28. 版本号 */
void Device::pGetVersion(quint64 id, QJsonObject params)
{
    Q_UNUSED(params)

    m_module->sendPostRequest("/settings/version", QJsonValue(), id, "GetVersion");
}

/* 29.工业机器人上下电 */
void Device::pSetIRControlMode(quint64 id, QJsonObject params)
{
    QJsonObject dataObj = params.value("data").toObject();

    checkStringValue(dataObj, "controlMode");

    m_module->sendPostRequest("/interface/controlMode", dataObj, id, "SetIRControlMode");
}


void Device::_sendStatus(quint64 id, const RobotStatus status, QString api)
{
    m_module->sendPostRequest("/protocol/exchange", status.getObjForSend(), id, api);
}

void Device::_getStatus(quint64 id, QString api)
{
    m_module->sendPostRequest("/protocol/exchange", m_status.getObjForSend(), id, api);
}

bool Device::checkIntValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isDouble()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

bool Device::checkFloatValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isDouble()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

bool Device::checkBoolValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isBool()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

bool Device::checkStringValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isString()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

bool Device::checkObjectValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isObject()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

bool Device::checkArrayValue(const QJsonObject &obj, const QString &value)
{
    if (!obj.contains(value)) {
        qDebug().noquote() << QString("'%1':missing").arg(value);
        return false;
    } else if (!obj.value(value).isArray()) {
        qWarning().noquote() << QString("'%1':type error").arg(value);
        return false;
    }
    return true;
}

/* 收到reply */
void Device::onReplyMessage_slot(QJsonValue value, QString url, quint64 id, QString api)
{
    bool isSelfhandle = false;

    if (value.isArray()) {
        QJsonArray arr = value.toArray();
    } else {
        QJsonObject obj = value.toObject();
        if (url.endsWith("/protocol/exchange")) {
            m_status.setStatus(obj);
        }

        isSelfhandle = hanleSelfFunction(url, api, id, obj);
    }

    if (isSelfhandle == false) {
        emit onReplyMessage_signal(id, value);
    }

    m_requestMap.remove(id);
}

bool Device::hanleSelfFunction(const QString url, const QString api, const quint64 id, const QJsonObject &obj)
{
    Q_UNUSED(url)

    if (api.contains("SearchDobot")) {
        handleSearchDobot(id, obj);
    } else if (api.contains("ConnectDobot")) {
        handleConnectDobot(id, obj);
    } else if (m_selfExchangeFunMap.contains(api)) {
        handleSelfExchange(api, id, obj);
    } else if (api.contains("SetDebuggerStart")) {
        // omit
    } else {
        return false;
    }
    return true;
}

void Device::handleSearchDobot(quint64 id, QJsonObject obj)
{
    if (obj.contains("value")) {
        QString status = obj.value("value").toString();
        QJsonArray resArray;
        if (status == "connected") {
            QJsonObject deviceObj;
            deviceObj.insert("portName", "192.168.5.1");
            if (m_isConnected) {
                deviceObj.insert("status", "connected");
            } else {
                deviceObj.insert("status", "unconnected");
            }
            resArray.append(deviceObj);
        }
        emit onReplyMessage_signal(id, resArray);
    }
}

void Device::handleConnectDobot(quint64 id, QJsonObject obj)
{
    if (obj.contains("value")) {
        QString status = obj.value("value").toString();
        if (status == "connected") {
//            m_wsPort =
            if (m_isConnected == false) {
                m_isConnected = true;
                m_modebug->udpOpen();
            }
            emit onReplyMessage_signal(id, true);
        }
    }
}

void Device::handleSelfExchange(QString api, quint64 id, QJsonObject obj)
{
    Q_UNUSED(obj)

    QString key = m_selfExchangeFunMap.value(api);
    QVariant value = m_status.getStatus(key);

    QJsonObject resObj;

    if (key.contains("toolCoordinate") or key.contains("userCoordinate")) {
        resObj.insert("value", value.toInt());
    } else if (key.contains("controlMode") or key.contains("coordinate")
               or key.contains("jogMode") or key.contains("autoManual")) {
        resObj.insert("value", value.toString());
    } else {
        resObj.insert("value", value.toJsonArray());
    }

    emit onReplyMessage_signal(id, resObj);
}

void Device::onModebugMessage_slot(QString msg)
{
    Q_UNUSED(msg)

//    qDebug() << "msg" << msg;
}

void Device::onModebugStateChanged_slot(Mobdebug::ModebugState state, quint64 id)
{
    qDebug() << "modebug state changed:" << state;
    if (state == Mobdebug::MODEBUG_LISTENING) {
        QTimer::singleShot(200, this, [=](){
            QJsonObject params = m_requestMap.value(id);
            m_module->sendPostRequest("/debugger/start", params.value("data").toObject(), id, "SetDebuggerStart");
        });
    }
}

void Device::onModebugFinish_slot(quint64 id)
{
    qDebug() << __FUNCTION__;
    emit onReplyMessage_signal(id, true);
}

void Device::onModebugErrorOccured_slot(quint64 id, int code)
{
    qDebug() << __FUNCTION__;
    emit onErrorOccured_signal(id, code);
}
