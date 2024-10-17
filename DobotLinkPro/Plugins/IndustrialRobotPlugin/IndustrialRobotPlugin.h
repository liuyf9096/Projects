#ifndef INDUSTRIALROBOTPLUGIN_H
#define INDUSTRIALROBOTPLUGIN_H

#include "DPluginInterface.h"

#include <QObject>
#include <QJsonObject>
#include <QMap>

#include "MessageCenter/DPacket.h"
#include "Device.h"

class IndustrialRobotPlugin : public DPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Dobot.plugin.interface")
    Q_INTERFACES(DPluginInterface)

public:
    static const QString PluginName;
    static const QString Version;

    IndustrialRobotPlugin(QObject *parent = nullptr);

    QString getVersion() override;

private:
    QMap<quint64, DRequestPacket> m_requestPacketMap;

    Device *m_device;

    void disconnectDobot(quint16 port = 0);
    void handleDobotLinkCommand(const QJsonObject &obj);
    void handleIndustrialRobotCommand(const QJsonObject &obj);

private slots:
    virtual void pReceiveMassage_slot(QString id, QJsonObject obj) override;

    void handleReplyMessage_slot(quint64 id, QJsonValue value);
    void handleErrorOccured_slot(quint64 id, int code);
};

#endif // INDUSTRIALROBOTPLUGIN_H
