#ifndef MAGICIANPLUGIN_H
#define MAGICIANPLUGIN_H

#include "DPluginInterface.h"

#include <QObject>
#include <QJsonObject>
#include "MagicianPacket.h"
#include "MagicianController.h"

class MagicianPlugin : public DPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Dobot.plugin.interface")
    Q_INTERFACES(DPluginInterface)

public:
    MagicianPlugin(QObject *parent = nullptr);
    static const QString PluginID;

private:
    MagicDeviceController *m_controller;

    void handleDobotLinkCommand(const QJsonObject &obj);
    void handleMagicianCommand(const QJsonObject &obj);

    bool checkPacket(const MagicianPacket &packet);
    void handleGetPoseCmd(const MagicianPacket &packet);
    void handleActionCmd(const MagicianPacket &packet);
    void handleStateCmd(const MagicianPacket &packet);
    void handleParamsCmd(const MagicianPacket &packet);
    void handleDobotCmd(const MagicianPacket &packet);
    void handleIOCmd(const MagicianPacket &packet);
    void handleDeviceCmd(const MagicianPacket &packet);
    void handleSensorCmd(const MagicianPacket &packet);
    void handleHHTTrigCmd(const MagicianPacket &packet);
    void handleOtherCmd(const MagicianPacket &packet);

signals:
    void closeAllDevice_signal();

private slots:
    virtual void pReceiveMassage_slot(QString id, QJsonObject obj);
    void handleDeviceDisconnected_slot(QString portName, quint16 wsPort);
    void sendResPacket_slot(QJsonObject resObj);
};

#endif // MAGICIANPLUGIN_H
