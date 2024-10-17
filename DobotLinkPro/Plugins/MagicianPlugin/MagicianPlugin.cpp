#include "MagicianPlugin.h"

#include <QDebug>

const QString MagicianPlugin::PluginID = "Magician";

MagicianPlugin::MagicianPlugin(QObject *parent) : DPluginInterface(parent)
{
    m_controller = new MagicDeviceController(this);
    connect(m_controller, &MagicDeviceController::deviceDisconnected_signal,
            this, &MagicianPlugin::handleDeviceDisconnected_slot);
    connect(this, &MagicianPlugin::closeAllDevice_signal,
            m_controller, &MagicDeviceController::closeAllDevice_slot);
    connect(m_controller, &MagicDeviceController::sendFinishPacket_signal,
            this, &MagicianPlugin::sendResPacket_slot);
}

/* 收到消息 */
void MagicianPlugin::pReceiveMassage_slot(QString id, QJsonObject obj)
{
    if (id == "ALL") {
        handleDobotLinkCommand(obj);
    } else if (id == PluginID) {
        handleMagicianCommand(obj);
    }
}

/* [!!!消息分发!!!] */
void MagicianPlugin::handleDobotLinkCommand(const QJsonObject &obj)
{
    qDebug() << "[ALL] {Magician} get obj" << obj;
    if (obj.contains("METHOD")) {
        QString method = obj.value("METHOD").toString();

        if (method == "EXIT") {
            emit closeAllDevice_signal();
        } else if (method == "CloseWebSocket") {

            QJsonObject params = obj.value("params").toObject();
            quint16 port = static_cast<quint16>(params.value("WSport").toInt(0));
            m_controller->disConnectDevices(port);
        }
    }
}

void MagicianPlugin::handleMagicianCommand(const QJsonObject &obj)
{
//    qDebug() << "{MagicianPlugin}" << obj;
    MagicianPacket packet;
    packet.setPacket(obj);

    if (checkPacket(packet) == false) {
        return;
    }

    //search & dis/connect
    if (packet.api.endsWith("Dobot")) {
        handleDobotCmd(packet);
        return;
    }

    if (!m_controller->isDeviceConnected(packet.portName)) {
        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "Dobot is NOT connected. Please connect first.");
        QJsonObject resObj = resPacket.getResultObj();
        emit pSendMessage_signal(PluginID, resObj);
        return;
    }

    if (packet.api.contains("GetPose")) {
        handleGetPoseCmd(packet);
    } else if (packet.api.endsWith("Cmd")) {         //execute command
        handleActionCmd(packet);
    } else if (packet.api.endsWith("State")) {       //get/clear state
        handleStateCmd(packet);
    } else if (packet.api.endsWith("Params")) {      //set/get params
        handleParamsCmd(packet);
    } else if (packet.api.contains("IO")) {          //IO control
        handleIOCmd(packet);
    } else if (packet.api.contains("Device")) {      //Device
        handleDeviceCmd(packet);
    } else if (packet.api.endsWith("Sensor")) {      //Sensor
        handleSensorCmd(packet);
    } else if (packet.api.contains("HHTTrig")) {     //HHTTrig
        handleHHTTrigCmd(packet);
    } else {
        handleOtherCmd(packet);
    }
}

bool MagicianPlugin::checkPacket(const MagicianPacket &packet)
{
    if (packet.api == "SearchDobot") {
        return true;
    }

    if (packet.portName.isEmpty()) {

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(10, "PortName was not specified.");
        emit pSendMessage_signal(PluginID, resPacket.getResultObj());
        return false;
    }
    return true;
}

//![1] GetPose
void MagicianPlugin::handleGetPoseCmd(const MagicianPacket &packet)
{  
    QJsonObject resObj;

    if (packet.api == "GetPose") {
        resObj = m_controller->pGetPose(packet);
    } else if (packet.api == "GetPoseL") {
        resObj = m_controller->pGetPoseL(packet);
    } else {
        qDebug() << "can not handle api with : GetPose";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : GetPose");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![2] execute Cmd
void MagicianPlugin::handleActionCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetHOMECmd") {
        resObj = m_controller->pSetHOMECmd(packet);
    } else if (packet.api == "SetPTPCmd") {
        resObj = m_controller->pSetPTPCmd(packet);
    } else if (packet.api == "SetRCmd") {
        resObj = m_controller->pSetRCmd(packet);
    } else if (packet.api == "SetPTPPOCmd") {
        resObj = m_controller->pSetPTPPOCmd(packet);
    } else if (packet.api == "SetPTPWithLCmd") {
        resObj = m_controller->pSetPTPWithLCmd(packet);
    } else if (packet.api == "SetLCmd") {
        resObj = m_controller->pSetLCmd(packet);
    } else if (packet.api == "SetPTPPOWithLCmd") {
        resObj = m_controller->pSetPTPPOWithLCmd(packet);
    } else if (packet.api == "SetJOGCmd") {
        resObj = m_controller->pSetJOGCmd(packet);
    } else if (packet.api == "SetCPCmd") {
        resObj = m_controller->pSetCPCmd(packet);
    } else if (packet.api == "SetCPLECmd") {
        resObj = m_controller->pSetCPLECmd(packet);
    } else if (packet.api == "SetQueuedCmd") {
        resObj = m_controller->pSetQueuedCmd(packet);
    } else if (packet.api == "SetLostStepCmd") {
        resObj = m_controller->pSetLostStepCmd(packet);
    } else if (packet.api == "SetARCCmd") {
        resObj = m_controller->pSetARCCmd(packet);
    } else if (packet.api == "SetCircleCmd") {
        resObj = m_controller->pSetCircleCmd(packet);
    } else if (packet.api == "SetWAITCmd") {
        resObj = m_controller->pSetWAITCmd(packet);
    } else if (packet.api == "SetTRIGCmd") {
        resObj = m_controller->pSetTRIGCmd(packet);
    } else {
        qDebug() << "can not handle api with : Cmd";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : Cmd");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![3] get/clear State
void MagicianPlugin::handleStateCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "GetAlarmsState") {
        resObj = m_controller->pGetAlarmsState(packet);
    } else if (packet.api == "ClearAllAlarmsState") {
        resObj = m_controller->pClearAllAlarmsState(packet);
    } else {
        qDebug() << "can not handle api with : State";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : State");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![4] set/get Params
void MagicianPlugin::handleParamsCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetHOMEParams") {
        resObj = m_controller->pSetHOMEParams(packet);
    } else if (packet.api == "GetHOMEParams") {
        resObj = m_controller->pGetHOMEParams(packet);
    } else if (packet.api == "SetEndEffectorParams") {
        resObj = m_controller->pSetEndEffectorParams(packet);
    } else if (packet.api == "GetEndEffectorParams") {
        resObj = m_controller->pGetEndEffectorParams(packet);
    } else if (packet.api == "SetPTPJointParams") {
        resObj = m_controller->pSetPTPJointParams(packet);
    } else if (packet.api == "GetPTPJointParams") {
        resObj = m_controller->pGetPTPJointParams(packet);
    } else if (packet.api == "SetJOGJointParams") {
        resObj = m_controller->pSetJOGJointParams(packet);
    } else if (packet.api == "GetJOGJointParams") {
        resObj = m_controller->pGetJOGJointParams(packet);
    } else if (packet.api == "SetJOGCoordinateParams") {
        resObj = m_controller->pSetJOGCoordinateParams(packet);
    } else if (packet.api == "GetJOGCoordinateParams") {
        resObj = m_controller->pGetJOGCoordinateParams(packet);
    } else if (packet.api == "SetJOGLParams") {
        resObj = m_controller->pSetJOGLParams(packet);
    } else if (packet.api == "GetJOGLParams") {
        resObj = m_controller->pGetJOGLParams(packet);
    } else if (packet.api == "SetJOGCommonParams") {
        resObj = m_controller->pSetJOGCommonParams(packet);
    } else if (packet.api == "GetJOGCommonParams") {
        resObj = m_controller->pGetJOGCommonParams(packet);
    } else if (packet.api == "SetPTPLParams") {
        resObj = m_controller->pSetPTPLParams(packet);
    } else if (packet.api == "GetPTPLParams") {
        resObj = m_controller->pGetPTPLParams(packet);
    } else if (packet.api == "SetCPParams") {
        resObj = m_controller->pSetCPParams(packet);
    } else if (packet.api == "GetCPParams") {
        resObj = m_controller->pGetCPParams(packet);
    } else if (packet.api == "SetCPParams") {
        resObj = m_controller->pSetCPParams(packet);
    } else if (packet.api == "GetCPParams") {
        resObj = m_controller->pGetCPParams(packet);
    } else if (packet.api == "SetPTPCommonParams") {
        resObj = m_controller->pSetPTPCommonParams(packet);
    } else if (packet.api == "GetPTPCommonParams") {
        resObj = m_controller->pGetPTPCommonParams(packet);
    } else if (packet.api == "SetLostStepParams") {
        resObj = m_controller->pSetLostStepParams(packet);
    } else if (packet.api == "SetPTPJumpParams") {
        resObj = m_controller->pSetPTPJumpParams(packet);
    } else if (packet.api == "GetPTPJumpParams") {
        resObj = m_controller->pGetPTPJumpParams(packet);
    } else if (packet.api == "SetPTPCoordinateParams") {
        resObj = m_controller->pSetPTPCoordinateParams(packet);
    } else if (packet.api == "GetPTPCoordinateParams") {
        resObj = m_controller->pGetPTPCoordinateParams(packet);
    } else if (packet.api == "SetARCParams") {
        resObj = m_controller->pSetARCParams(packet);
    } else if (packet.api == "GetARCParams") {
        resObj = m_controller->pGetARCParams(packet);
    } else if (packet.api == "SetPTPJump2Params") {
        resObj = m_controller->pSetPTPJump2Params(packet);
    } else if (packet.api == "GetPTPJump2Params") {
        resObj = m_controller->pGetPTPJump2Params(packet);
    } else {
        qDebug() << "can not handle api with : Params";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : Params");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![5] search & dis/connect
void MagicianPlugin::handleDobotCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SearchDobot") {
        resObj = m_controller->pSearchDobot(packet);
    } else if (packet.api == "ConnectDobot") {
        resObj = m_controller->pConnectDobot(packet);
    } else if (packet.api == "DisconnectDobot") {
        resObj = m_controller->pDisConnectDobot(packet);
    } else {
        qDebug() << "can not handle api with : Dobot";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : Dobot");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![6] IO Cmd
void MagicianPlugin::handleIOCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetIOMultiplexing") {
        resObj = m_controller->pSetIOMultiplexing(packet);
    } else if (packet.api == "GetIOMultiplexing") {
        resObj = m_controller->pGetIOMultiplexing(packet);
    } else if (packet.api == "SetIODO") {
        resObj = m_controller->pSetIODO(packet);
    } else if (packet.api == "GetIODO") {
        resObj = m_controller->pGetIODO(packet);
    } else if (packet.api == "SetIOPWM") {
        resObj = m_controller->pSetIOPWM(packet);
    } else if (packet.api == "GetIOPWM") {
        resObj = m_controller->pGetIOPWM(packet);
    } else if (packet.api == "GetIODI") {
        resObj = m_controller->pGetIODI(packet);
    } else if (packet.api == "GetIOADC") {
        resObj = m_controller->pGetIOADC(packet);
    } else {
        qDebug() << "can not handle api with : IO";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : IO");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![7] Device Cmd
void MagicianPlugin::handleDeviceCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetDeviceSN") {
        resObj = m_controller->pSetDeviceSN(packet);
    } else if (packet.api == "GetDeviceSN") {
        resObj = m_controller->pGetDeviceSN(packet);
    } else if (packet.api == "SetDeviceName") {
        resObj = m_controller->pSetDeviceName(packet);
    } else if (packet.api == "GetDeviceName") {
        resObj = m_controller->pGetDeviceName(packet);
    } else if (packet.api == "GetDeviceVersion") {
        resObj = m_controller->pGetDeviceVersion(packet);
    } else if (packet.api == "SetDeviceWithL") {
        resObj = m_controller->pSetDeviceWithL(packet);
    } else if (packet.api == "GetDeviceWithL") {
        resObj = m_controller->pGetDeviceWithL(packet);
    } else if (packet.api == "GetDeviceTime") {
        resObj = m_controller->pGetDeviceTime(packet);
    } else {
        qDebug() << "can not handle api with : Device";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : Device");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![8] Sensor Cmd
void MagicianPlugin::handleSensorCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetInfraredSensor") {
        resObj = m_controller->pSetInfraredSensor(packet);
    } else if (packet.api == "GetInfraredSensor") {
        resObj = m_controller->pGetInfraredSensor(packet);
    } else if (packet.api == "SetColorSensor") {
        resObj = m_controller->pSetColorSensor(packet);
    } else if (packet.api == "GetColorSensor") {
        resObj = m_controller->pGetColorSensor(packet);
    } else {
        qDebug() << "can not handle api with : Sensor";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : Sensor");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![9] HHTTrig Cmd
void MagicianPlugin::handleHHTTrigCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetHHTTrigMode") {
        resObj = m_controller->pSetHHTTrigMode(packet);
    } else if (packet.api == "GetHHTTrigMode") {
        resObj = m_controller->pGetHHTTrigMode(packet);
    } else if (packet.api == "SetHHTTrigOutputEnabled") {
        resObj = m_controller->pSetHHTTrigOutputEnabled(packet);
    } else if (packet.api == "GetHHTTrigOutputEnabled") {
        resObj = m_controller->pGetHHTTrigOutputEnabled(packet);
    } else if (packet.api == "GetHHTTrigOutput") {
        resObj = m_controller->pGetHHTTrigOutput(packet);
    } else {
        qDebug() << "can not handle api with : HHTTrig";

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle api with : HHTTrig");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

//![10] Other Cmd
void MagicianPlugin::handleOtherCmd(const MagicianPacket &packet)
{
    QJsonObject resObj;

    if (packet.api == "SetCmdTimeout") {
        resObj = m_controller->pSetCmdTimeout(packet);
    } else if (packet.api == "SetEMotor") {
        resObj = m_controller->pSetEMotor(packet);
    } else if (packet.api == "SetEMotorS") {
        resObj = m_controller->pSetEMotorS(packet);
    } else if (packet.api == "SetEndEffectorSuctionCup") {
        resObj = m_controller->pSetEndEffectorSuctionCup(packet);
    } else if (packet.api == "GetEndEffectorSuctionCup") {
        resObj = m_controller->pGetEndEffectorSuctionCup(packet);
    } else if (packet.api == "SetEndEffectorGripper") {
        resObj = m_controller->pSetEndEffectorGripper(packet);
    } else if (packet.api == "GetEndEffectorGripper") {
        resObj = m_controller->pGetEndEffectorGripper(packet);
    } else if (packet.api == "SetEndEffectorLaser") {
        resObj = m_controller->pSetEndEffectorLaser(packet);
    } else if (packet.api == "GetEndEffectorLaser") {
        resObj = m_controller->pGetEndEffectorLaser(packet);
    } else {
        qDebug() << "can not handle this api :( , please contact the author. api:" << packet.api;

        MagicianResPacket resPacket(packet.id, packet.port);
        resPacket.setErrorObj(82, "can not handle this api :( , please contact the author.");
        resObj = resPacket.getResultObj();
    }

    if (!resObj.isEmpty()) {
        resObj.insert("id", packet.id);
        emit pSendMessage_signal(PluginID, resObj);
    }
}

/* SLOT */
void MagicianPlugin::handleDeviceDisconnected_slot(QString portName, quint16 wsPort)
{
    MagicianPacket resPacket;
    resPacket.port = wsPort;
    resPacket.method = "dobotlink." + PluginID + ".disconnected";
    resPacket.paramsObj.insert("portName", portName);

    emit pSendMessage_signal(PluginID, resPacket.getNotificationObj());
}

void MagicianPlugin::sendResPacket_slot(QJsonObject resObj)
{
    emit pSendMessage_signal(PluginID, resObj);
}
