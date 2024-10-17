#ifndef DOWNLOADPLUGIN_H
#define DOWNLOADPLUGIN_H

#include "DPluginInterface.h"

#include <QObject>
#include <QJsonObject>
#include <QMap>

#include "MessageCenter/DPacket.h"
#include "DDfufile.h"

class DownloadPlugin : public DPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Dobot.plugin.interface")
    Q_INTERFACES(DPluginInterface)

public:
    static const QString PluginName;
    static const QString Version;

    DownloadPlugin(QObject *parent = nullptr);

    QString getVersion() override;

private:
    DDfufile *m_dfufile;
    quint64 m_handlingId;
    quint16 m_handlingWsPort;
    QMap<quint64, DRequestPacket> m_requestPacketMap;

    void handleDfuDownloadCmd(const QJsonObject &obj);

private slots:
    virtual void pReceiveMassage_slot(QString id, QJsonObject obj) override;
    void onFinish_slot(bool ok);
};

#endif // DOWNLOADPLUGIN_H
