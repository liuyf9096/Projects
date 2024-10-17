#ifndef MAGICIANCONTROLLER_H
#define MAGICIANCONTROLLER_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QTimer>
#include "MagicianPacket.h"
#include "MWaitForFinish.h"

class DeviceInfo {
public:
    DeviceInfo();

    int id;
    bool isConnected;
    QString status;

    QString portName;
    QString SN;
    QString name;

    double posX;
    double posY;
    double posZ;
    double baseArc;
    double bigArmArc;
    double littleArmArc;
};

class MagicDeviceController : public QObject
{
    Q_OBJECT
public:
    explicit MagicDeviceController(QObject *parent = nullptr);

    QMap<QString, DeviceInfo> m_DevInfoMap;
    QMap<QString, quint16> m_PortnamePortMap;
    QMap<MWaitForFinish*, int> m_waitFinishMap;

    bool isDeviceConnected(QString portName);
    void updataDeviceState();
    void disConnectDevices(quint16 port);
    bool isDeviceAvailable(QString portName);

    QJsonObject getCmdResultObj(const MagicianPacket &packet, const int res = 0, QJsonValue data = QJsonValue());
    QJsonObject getConnectResultObj(const MagicianPacket &packet, const int res = 0);

    /* 1.2 指令超时 */
    QJsonObject pSetCmdTimeout(const MagicianPacket &packet);

    /* 1.3 连接/断开连接 */
    QJsonObject pSearchDobot(const MagicianPacket &packet);

    QJsonObject pConnectDobot(const MagicianPacket &packet);
    QJsonObject pDisConnectDobot(const MagicianPacket &packet);
    void pDisConnectDobot(const QString &portName);

    /* 1.4 指令队列控制 */
    QJsonObject pSetQueuedCmd(const MagicianPacket &packet);

    /* 1.5 设备信息 */
    QJsonObject pSetDeviceSN(const MagicianPacket &packet);
    QJsonObject pGetDeviceSN(const MagicianPacket &packet);

    QJsonObject pSetDeviceName(const MagicianPacket &packet);
    QJsonObject pGetDeviceName(const MagicianPacket &packet);

    QJsonObject pGetDeviceVersion(const MagicianPacket &packet);

    QJsonObject pSetDeviceWithL(const MagicianPacket &packet);  //滑轨
    QJsonObject pGetDeviceWithL(const MagicianPacket &packet);

    QJsonObject pGetDeviceTime(const MagicianPacket &packet);

    /* 1.6 实时位姿 */
    QJsonObject pGetPose(const MagicianPacket &packet);
    QJsonObject pGetPoseL(const MagicianPacket &packet);

    /* 1.7 报警功能 */
    QJsonObject pGetAlarmsState(const MagicianPacket &packet);
    QJsonObject pClearAllAlarmsState(const MagicianPacket &packet);

    /* 1.8 回零功能 */
    QJsonObject pSetHOMECmd(const MagicianPacket &packet);

    QJsonObject pSetHOMEParams(const MagicianPacket &packet);
    QJsonObject pGetHOMEParams(const MagicianPacket &packet);

    /* 1.9 HHT 手持示教器 */
    QJsonObject pSetHHTTrigMode(const MagicianPacket &packet);
    QJsonObject pGetHHTTrigMode(const MagicianPacket &packet);

    QJsonObject pSetHHTTrigOutputEnabled(const MagicianPacket &packet);
    QJsonObject pGetHHTTrigOutputEnabled(const MagicianPacket &packet);

    QJsonObject pGetHHTTrigOutput(const MagicianPacket &packet);

    /* 1.10 末端执行器 */
    QJsonObject pSetEndEffectorParams(const MagicianPacket &packet);    //1.设置末端坐标偏移量
    QJsonObject pGetEndEffectorParams(const MagicianPacket &packet);    //2.获取末端坐标偏移量

    QJsonObject pSetEndEffectorLaser(const MagicianPacket &packet);     //3.设置激光状态
    QJsonObject pGetEndEffectorLaser(const MagicianPacket &packet);     //4.获取激光状态

    QJsonObject pSetEndEffectorSuctionCup(const MagicianPacket &packet);//5.设置气泵状态
    QJsonObject pGetEndEffectorSuctionCup(const MagicianPacket &packet);//6.获取气泵状态

    QJsonObject pSetEndEffectorGripper(const MagicianPacket &packet);   //7.设置夹爪状态
    QJsonObject pGetEndEffectorGripper(const MagicianPacket &packet);   //8.获取夹爪状态

    /* 1.11 JOG 功能 */
    QJsonObject pSetJOGJointParams(const MagicianPacket &packet);
    QJsonObject pGetJOGJointParams(const MagicianPacket &packet);

    QJsonObject pSetJOGCoordinateParams(const MagicianPacket &packet);
    QJsonObject pGetJOGCoordinateParams(const MagicianPacket &packet);

    QJsonObject pSetJOGLParams(const MagicianPacket &packet);
    QJsonObject pGetJOGLParams(const MagicianPacket &packet);

    QJsonObject pSetJOGCommonParams(const MagicianPacket &packet);
    QJsonObject pGetJOGCommonParams(const MagicianPacket &packet);

    QJsonObject pSetJOGCmd(const MagicianPacket &packet);

    /* 1.12 PTP 功能 */
    QJsonObject pSetPTPJointParams(const MagicianPacket &packet);       //1.设置各关节坐标轴的速度和加速度
    QJsonObject pGetPTPJointParams(const MagicianPacket &packet);       //2.获取各关节坐标轴的速度和加速度

    QJsonObject pSetPTPCoordinateParams(const MagicianPacket &packet);  //3.设置各笛卡尔坐标轴的速度和加速度
    QJsonObject pGetPTPCoordinateParams(const MagicianPacket &packet);  //4.获取各笛卡尔坐标轴的速度和加速度

    QJsonObject pSetPTPJumpParams(const MagicianPacket &packet);        //5.设置JUMP模式下抬升高度和最大抬升高度
    QJsonObject pGetPTPJumpParams(const MagicianPacket &packet);        //6.获取JUMP模式下抬升高度和最大抬升高度

    QJsonObject pSetPTPJump2Params(const MagicianPacket &packet);       //7.设置JUMP模式下扩展参数
    QJsonObject pGetPTPJump2Params(const MagicianPacket &packet);       //8.获取JUMP模式下扩展参数

    QJsonObject pSetPTPLParams(const MagicianPacket &packet);           //9.设置滑轨速度和加速度
    QJsonObject pGetPTPLParams(const MagicianPacket &packet);           //10.获取滑轨速度和加速度

    QJsonObject pSetPTPCommonParams(const MagicianPacket &packet);      //11.设置运动的速度百分比和加速度百分比
    QJsonObject pGetPTPCommonParams(const MagicianPacket &packet);      //12.获取运动的速度百分比和加速度百分比

    QJsonObject pSetPTPCmd(const MagicianPacket &packet);               //13.执行 PTP 指令
    QJsonObject pSetRCmd(const MagicianPacket &packet);                 //13.执行 PTP 指令(设置R角度)
    QJsonObject pSetPTPPOCmd(const MagicianPacket &packet);             //14.执行带 I/O 控制的 PTP 指令
    QJsonObject pSetPTPWithLCmd(const MagicianPacket &packet);          //15.执行带滑轨的 PTP 指令
    QJsonObject pSetLCmd(const MagicianPacket &packet);                 //15.执行带滑轨的 PTP 指令
    QJsonObject pSetPTPPOWithLCmd(const MagicianPacket &packet);        //16.执行带 I/O 控制 和 滑轨 的 PTP 指令

    /* 1.13 CP 功能 */
    QJsonObject pSetCPParams(const MagicianPacket &packet);
    QJsonObject pGetCPParams(const MagicianPacket &packet);

    QJsonObject pSetCPCmd(const MagicianPacket &packet);
    QJsonObject pSetCPLECmd(const MagicianPacket &packet);

    /* 1.14 ARC 功能 */
    QJsonObject pSetARCParams(const MagicianPacket &packet);
    QJsonObject pGetARCParams(const MagicianPacket &packet);

    QJsonObject pSetARCCmd(const MagicianPacket &packet);
    QJsonObject pSetCircleCmd(const MagicianPacket &packet);

    /* 1.15 丢步检测 */
    QJsonObject pSetLostStepParams(const MagicianPacket &packet);
    QJsonObject pSetLostStepCmd(const MagicianPacket &packet);

    /* 1.16 WAIT 功能 */
    QJsonObject pSetWAITCmd(const MagicianPacket &packet);
    QJsonObject pSetTRIGCmd(const MagicianPacket &packet);

    /* 1.17 EIO 功能 */
    QJsonObject pSetIOMultiplexing(const MagicianPacket &packet);       //1.设置 I/O 复用
    QJsonObject pGetIOMultiplexing(const MagicianPacket &packet);       //2.读取 I/O 复用

    QJsonObject pSetIODO(const MagicianPacket &packet);                 //3.设置 I/O 输出电平
    QJsonObject pGetIODO(const MagicianPacket &packet);                 //4.读取 I/O 输出电平

    QJsonObject pSetIOPWM(const MagicianPacket &packet);                //5.设置 PWM 输出
    QJsonObject pGetIOPWM(const MagicianPacket &packet);                //6.读取 PWM 输出

    QJsonObject pGetIODI(const MagicianPacket &packet);                 //7.读取 I/O 输入电平
    QJsonObject pGetIOADC(const MagicianPacket &packet);                //8.读取 A/D 输入

    QJsonObject pSetEMotor(const MagicianPacket &packet);               //9.设置扩展电机速度
    QJsonObject pSetEMotorS(const MagicianPacket &packet);              //10.设置扩展电机速度和移动距离

    QJsonObject pSetInfraredSensor(const MagicianPacket &packet);       //11.使能光电传感器
    QJsonObject pGetInfraredSensor(const MagicianPacket &packet);       //12.获取光电传感器读数

    QJsonObject pSetColorSensor(const MagicianPacket &packet);          //13.使能颜色传感器
    QJsonObject pGetColorSensor(const MagicianPacket &packet);          //14.获取颜色传感器读数

signals:
    void deviceDisconnected_signal(QString portName, quint16 wsPort);
    void sendFinishPacket_signal(QJsonObject resObj);

public slots:
    void closeAllDevice_slot();
    void waitForFinish_slot(int res);

private:
    int m_ConnectedDeviceCount;
    QTimer *checkStateTimer;

    int _getConnecteDeviceCount();

private slots:
    void handleCheckTimeout_slot();
};

#endif // MAGICIANCONTROLLER_H
