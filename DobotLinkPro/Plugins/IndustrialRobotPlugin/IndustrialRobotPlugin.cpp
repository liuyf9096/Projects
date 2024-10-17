#include "IndustrialRobotPlugin.h"

#include <QJsonArray>
#include <QList>
#include <QUdpSocket>
#include <QTime>
#include <QDebug>

#define DEVICE_DLL_ERROR_BASE (120)

const QString IndustrialRobotPlugin::PluginName = "IndustrialRobot";
const QString IndustrialRobotPlugin::Version = "1.0.0";

IndustrialRobotPlugin::IndustrialRobotPlugin(QObject *parent) : DPluginInterface(parent)
{
    m_device = new Device(this);
    connect(m_device, &Device::onReplyMessage_signal,
            this, &IndustrialRobotPlugin::handleReplyMessage_slot);
    connect(m_device, &Device::onErrorOccured_signal,
            this, &IndustrialRobotPlugin::handleErrorOccured_slot);
}

QString IndustrialRobotPlugin::getVersion()
{
    return Version;
}

/* 收到消息 */
void IndustrialRobotPlugin::pReceiveMassage_slot(QString id, QJsonObject obj)
{
    if (id == "ALL") {
        handleDobotLinkCommand(obj);
    } else if (id.contains(PluginName)) {
        handleIndustrialRobotCommand(obj);
    }
}

void IndustrialRobotPlugin::handleReplyMessage_slot(quint64 id, QJsonValue value)
{
    DRequestPacket packet = m_requestPacketMap.value(id);
    DResultPacket resPacket(packet);

    emit pSendMessage_signal(PluginName, resPacket.getResultObj(value));
}

void IndustrialRobotPlugin::handleErrorOccured_slot(quint64 id, int code)
{
    DRequestPacket packet = m_requestPacketMap.value(id);
    DResultPacket resPacket(packet);

    qDebug() << "error id" << id << code;
    emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_NUKNOWN_ERROR));
}

/* [!!!消息分发!!!] */
void IndustrialRobotPlugin::handleDobotLinkCommand(const QJsonObject &obj)
{
    qDebug() << "[ALL] {IndustrialRobotPlugin} get obj" << obj;

    if (obj.contains("METHOD")) {
        QString method = obj.value("METHOD").toString();

        if (method == "EXIT") {
            //... close all device...
            disconnectDobot();
        } else if (method == "CloseWebSocket") {
            QJsonObject params = obj.value("params").toObject();
            quint16 port = static_cast<quint16>(params.value("WSport").toInt());
            disconnectDobot(port);
        }
    }
}

void IndustrialRobotPlugin::disconnectDobot(quint16 port)
{
    qDebug() << "close robot device, port:" << port;
    m_device->pDisconnectDobot();
}

void IndustrialRobotPlugin::handleIndustrialRobotCommand(const QJsonObject &obj)
{
//    qDebug() << "get obj" << obj;

    DRequestPacket packet;
    packet.setPacketFromObj(obj);
    DResultPacket resPacket(packet);

    m_requestPacketMap.insert(packet.id, packet);

    if (packet.api == "ConnectDobot") {
        if (packet.getParamValue("portName").toString().isEmpty()) {
            m_requestPacketMap.remove(packet.id);

            emit pSendMessage_signal(PluginName, resPacket.getErrorObjWithCode(ERROR_INVALID_PORTNAME));
            return;
        }
        m_device->setWsPort(packet.wsPort);
    }

    m_device->sendCommand(packet.api, packet.id, packet.paramsObj);

    return;
}

