#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QMap>

#include "RobotStatus.h"
#include "Mobdebug.h"

class Module;
class Device : public QObject
{
    Q_OBJECT
public:
    typedef void (Device::*APIFunction)(quint64, QJsonObject);

    explicit Device(QObject *parent = nullptr);

    void pSearchDobot(quint64 id, QJsonObject params);
    void pConnectDobot(quint64 id, QJsonObject params);
    void pDisconnectDobot(quint64 id = 0, QJsonObject params = QJsonObject());

    void sendCommand(QString api, quint64 id, QJsonObject params);

    void setWsPort(quint16 port);

private:
    /* 3. 连接状态 */
    void pGetConnectionState(quint64 id, QJsonObject params);

    /* 万能函数 */
    void pSendPostCmd(quint64 id, QJsonObject params);
    void pSendGetCmd(quint64 id, QJsonObject params);
    void pReadFile(quint64 id, QJsonObject params);
    void pWriteFile(quint64 id, QJsonObject params);

    /* 4. 基础数据交互 */
    void pExchange(quint64 id, QJsonObject params);

    void pGetDobotStatus(quint64 id, QJsonObject params);

    void pSetControlMode(quint64 id, QJsonObject params);
    void pGetControlMode(quint64 id, QJsonObject params);

    void pSetCoordinateMode(quint64 id, QJsonObject params);
    void pGetCoordinateMode(quint64 id, QJsonObject params);

    void pSetJogMode(quint64 id, QJsonObject params);
    void pGetJogMode(quint64 id, QJsonObject params);

    void pSetToolCoordPattern(quint64 id, QJsonObject params);
    void pGetToolCoordPattern(quint64 id, QJsonObject params);

    void pSetUserCoordPattern(quint64 id, QJsonObject params);
    void pGetUserCoordPattern(quint64 id, QJsonObject params);

    void pGetAutoManualMode(quint64 id, QJsonObject params);

    void pGetJointCoordinate(quint64 id, QJsonObject params);
    void pGetCartesianCoordinate(quint64 id, QJsonObject params);
    void pGetAlarms(quint64 id, QJsonObject params);
    void pClearAlarms(quint64 id, QJsonObject params);
    void pGetInput(quint64 id, QJsonObject params);
    void pSetOutput(quint64 id, QJsonObject params);
    void pGetOutput(quint64 id, QJsonObject params);

    /* 5.2 全局比例 */
    void pSetCommonSetting(quint64 id, QJsonObject params);
    void pGetCommonSetting(quint64 id, QJsonObject params);

    /* 5.3 示教参数 */
    void pSetTeachJoint(quint64 id, QJsonObject params);
    void pGetTeachJoint(quint64 id, QJsonObject params);

    void pSetTeachCoordinate(quint64 id, QJsonObject params);
    void pGetTeachCoordinate(quint64 id, QJsonObject params);

    /* 5.4 再现参数 */
    void pSetPlaybackJoint(quint64 id, QJsonObject params);
    void pGetPlaybackJoint(quint64 id, QJsonObject params);

    void pSetPlaybackCoordinate(quint64 id, QJsonObject params);
    void pGetPlaybackCoordinate(quint64 id, QJsonObject params);

    void pSetPlaybackArch(quint64 id, QJsonObject params);
    void pGetPlaybackArch(quint64 id, QJsonObject params);

    /* 5.5~5.6 坐标系 */
    void pSetToolCoordinate(quint64 id, QJsonObject params);
    void pGetToolCoordinate(quint64 id, QJsonObject params);
    void pSetUserCoordinate(quint64 id, QJsonObject params);
    void pGetUserCoordinate(quint64 id, QJsonObject params);

    /* 5.7 示教点列表 */
    void pSetTeachFileUpdate(quint64 id, QJsonObject params);
    void pDelTeachFileDelete(quint64 id, QJsonObject params);

    /* 6.1 工具坐标系标定 */
    void pSetCalibrateToolCoor(quint64 id, QJsonObject params);
    void pSetCalibrateToolPosition(quint64 id, QJsonObject params);
    void pSetCalibrateToolPose(quint64 id, QJsonObject params);

    /* 6.2 用户坐标系标定 */
    void pSetCalibrateUserCoor(quint64 id, QJsonObject params);

    /* 6.3 左右手标定 */
    void pSetCalibrateLeftRightHand(quint64 id, QJsonObject params);

    /* 6.4 回零 */
    void pSetCalibrateHome(quint64 id, QJsonObject params);

    /* 7.1~7.10 调试器 */
    void pGetDebuggerState(quint64 id, QJsonObject params);
    void pSetDebuggerStart(quint64 id, QJsonObject params);
    void pSetDebuggerRun(quint64 id, QJsonObject params);
    void pSetDebuggerStop(quint64 id, QJsonObject params);
    void pSetDebuggerSuspend(quint64 id, QJsonObject params);
    void pSetDebuggerStepIn(quint64 id, QJsonObject params);
    void pSetDebuggerStepOver(quint64 id, QJsonObject params);
    void pSetDebuggerSetb(quint64 id, QJsonObject params);
    void pSetDebuggerDelb(quint64 id, QJsonObject params);
    void pSetDebuggerDelAllb(quint64 id, QJsonObject params);

    /* 8. 面板模拟 */
    void pSetAutoManualMode(quint64 id, QJsonObject params);
    void pSetThreeSwitch(quint64 id, QJsonObject params);  
    void pSetEmergencyStop(quint64 id, QJsonObject params);
    void pSetJogCmd(quint64 id, QJsonObject params);

    /* 9. 网络设置 */
    void pSetEthernet(quint64 id, QJsonObject params);
    void pGetEthernet(quint64 id, QJsonObject params);

    /* 10.1 获取相机坐标 */
    void pSetVisionCoordinate(quint64 id, QJsonObject params);

    /* 10.2 运动到指定点 */
    void pSetMoveCmd(quint64 id, QJsonObject params);

    /* 11. 传送带跟踪 */
    void pGetCurrentEncoder(quint64 id, QJsonObject params);
    void pGetSensorEncoder(quint64 id, QJsonObject params);
    void pSetSensorEncoderListen(quint64 id, QJsonObject params);

    /* 12. 手动模式中全局速度比例 */
    void pSetManualSpeedRatio(quint64 id, QJsonObject params);
    void pGetManualSpeedRatio(quint64 id, QJsonObject params);

    /* 13. DI 模拟输入 */
    void pSetDIMode(quint64 id, QJsonObject params);
    void pGetDIMode(quint64 id, QJsonObject params);
    void pSetDIValue(quint64 id, QJsonObject params);

    /* 14. 通用IO模拟量 */
    void pSetGPIOAO(quint64 id, QJsonObject params);
    void pGetGPIOAO(quint64 id, QJsonObject params);
    void pSetGPIOAI(quint64 id, QJsonObject params);
    void pGetGPIOAI(quint64 id, QJsonObject params);

    /* 15. 协作机器人安全配置 */
    void pSetGeneralSafeSetting(quint64 id, QJsonObject params);
    void pSetLoadParams(quint64 id, QJsonObject params);
    void pSetAdvancedFunc(quint64 id, QJsonObject params);
    void pSetCollisionDetect(quint64 id, QJsonObject params);
    void pSetSafeParams(quint64 id, QJsonObject params);

    /* 16. 电子皮肤 */
    void pSetElecSkinEnable(quint64 id, QJsonObject params);
    void pSetElecSkinReset(quint64 id, QJsonObject params);
    void pGetElecSkinReset(quint64 id, QJsonObject params);
    void pSetElecSkinParams(quint64 id, QJsonObject params);
    void pGetElecSkinParams(quint64 id, QJsonObject params);

    /* 17. 回零 */
    void pSetGoHomeCmd(quint64 id, QJsonObject params);

    /* 18. 开关抱闸 */
    void pSetAxisJointBrake(quint64 id, QJsonObject params);
    void pGetAxisJointBrake(quint64 id, QJsonObject params);

    /* 19. 轨迹复现 */
    void pSetRecurrentTrack(quint64 id, QJsonObject params);
    void pSetDebugReTrace(quint64 id, QJsonObject params);
    void pGetDebugReTrace(quint64 id, QJsonObject params);

    /* 20. 协作机器人自动识别 */
    void pSetAutoIdentify(quint64 id, QJsonObject params);

    /* 21.协作机器人安装位置 */
    void pSetInstallPosture(quint64 id, QJsonObject params);

    /* 22.协作机器人实轴、虚轴 */
    void pSetSimulatedAxies(quint64 id, QJsonObject params);

    /* 23.协作机器人末端执行器 */
    void pSetRobottiqGripperEnable(quint64 id, QJsonObject params);
    void pGetRobottiqGripperEnable(quint64 id, QJsonObject params);
    void pSetRobottiqGripperEnableK(quint64 id, QJsonObject params);
    void pSetHitbotGripperEnable(quint64 id, QJsonObject params);
    void pGetHitbotGripperEnable(quint64 id, QJsonObject params);
    void pSetHitbotGripper(quint64 id, QJsonObject params);
    void pSetDHGripperEnable(quint64 id, QJsonObject params);
    void pGetDHGripperEnable(quint64 id, QJsonObject params);
    void pSetDHGripper(quint64 id, QJsonObject params);
    void pGetDHGripper(quint64 id, QJsonObject params);

    /* 24.协作机器人 6维力传感器接口 */
    void pSetRobotiqSixForce(quint64 id, QJsonObject params);
    void pGetRobotiqSixForce(quint64 id, QJsonObject params);

    /* 24.2 模拟量接口 */
    void pSetEndAI(quint64 id, QJsonObject params);
    void pGetEndAI(quint64 id, QJsonObject params);

    /* 25.协作控制器上下电 */
    void pSetPowerControl(quint64 id, QJsonObject params);
    void pGetPowerControl(quint64 id, QJsonObject params);

    /* 26. 协作控制器上下电 */
    void pSetCRControlMode(quint64 id, QJsonObject params);

    /* 27.协作机器人RUNTO */
    void pSetCRRunTo(quint64 id, QJsonObject params);

    /* 28. 版本号 */
    void pGetVersion(quint64 id, QJsonObject params);

    /* 29.工业机器人上下电 */
    void pSetIRControlMode(quint64 id, QJsonObject params);

signals:
    void onReplyMessage_signal(quint64 id, QJsonValue value = QJsonValue());
    void onErrorOccured_signal(quint64 id, int code);

private:
    RobotStatus m_status;
    Module *m_module;
    Mobdebug *m_modebug;
    bool m_isConnected;
    QMap<QString, QString> m_selfExchangeFunMap;

    QMap<QString, APIFunction> m_FuncMap;
    QMap<quint64, QJsonObject> m_requestMap;

    quint16 m_wsPort;

    void _apiFunctionInit();
    void _selfExchangeFunMapInit();

    void _sendStatus(quint64 id, const RobotStatus status, QString api);
    void _getStatus(quint64 id, QString api);

    inline bool checkIntValue(const QJsonObject &obj, const QString &value);
    inline bool checkFloatValue(const QJsonObject &obj, const QString &value);
    inline bool checkBoolValue(const QJsonObject &obj, const QString &value);
    inline bool checkStringValue(const QJsonObject &obj, const QString &value);
    inline bool checkObjectValue(const QJsonObject &obj, const QString &value);
    inline bool checkArrayValue(const QJsonObject &obj, const QString &value);

    bool hanleSelfFunction(const QString url, const QString api, const quint64 id, const QJsonObject &obj);

    void handleSearchDobot(quint64 id, QJsonObject obj);
    void handleConnectDobot(quint64 id, QJsonObject obj);

    void handleSelfExchange(QString api, quint64 id, QJsonObject obj);

private slots:
    void onModebugMessage_slot(QString msg);
    void onModebugStateChanged_slot(Mobdebug::ModebugState state, quint64 id = 0);
    void onModebugFinish_slot(quint64 id);
    void onModebugErrorOccured_slot(quint64 id, int code);

    void onReplyMessage_slot(QJsonValue value, QString url, quint64 id, QString api);
};

#endif // DEVICE_H
