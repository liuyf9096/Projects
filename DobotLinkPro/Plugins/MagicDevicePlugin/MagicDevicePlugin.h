#ifndef MAGICDEVICEPLUGIN_H
#define MAGICDEVICEPLUGIN_H

#include "DPluginInterface.h"

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMap>

#include "MessageCenter/DPacket.h"
#include "MagicDevice.h"

class DeviceInfo {
public:
    QString portName;
    QString description;
    QString status;
};

class MagicDevice;
class DProgramDownload;
class QUdpSocket;

class MagicDevicePlugin : public DPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Dobot.plugin.interface")
    Q_INTERFACES(DPluginInterface)

public:
    static const QString PluginName;
    static const QString Version;

    MagicDevicePlugin(QObject *parent = nullptr);

    QString getVersion() override;

private:
    DProgramDownload *downloader;
    QMap<quint64, DRequestPacket> m_requestPacketMap;
    QMap<QString, MagicDevice *> m_deviceMap;
    QMap<QString, MagicDevice *> m_checkDeviceMap;
    bool m_researchFileter;
    QStringList m_portNameList;
    quint64 m_handlingid;
    QUdpSocket *m_udpSocket = nullptr;
    QMap<QString, QString> m_ipAndBroadcastMap;
    QString m_localIp;
    QMap<QString, QString> m_m1deviceMap;
    QMap<QString, DeviceInfo> m_preDeviceMap;
    bool m_isSearchingDevices;
    QStringList m_handlingSearchDeviceList;
    QTimer *m_searchTimer;

    void handleDobotLinkCommand(const QJsonObject &obj);
    void handleMagicDeviceCommand(const QJsonObject &obj);

    MagicDevice *_getDevice(const DRequestPacket &packet);

    void _checkDeviceWithType(QString portName, DeviceType type, quint64 id = 0);
    void _getAvailableSerialPort(QStringList filter = QStringList());
    QJsonArray _getSearchResult();

    void pSearchDobot(const DRequestPacket &packet);
    void pConnectDobot(const DRequestPacket &packet);
    void pDisconnectDobot(MagicDevice *device, const DRequestPacket &packet);
    void pDisconnectDobot(quint16 wsport);

    bool _checkActionApi(QString api);
    int _getDefaultTimeoutValue(QString devideType, QString cmd);
    bool handleActionCmd(MagicDevice *device, const DRequestPacket &packet);

    bool _checkQueueApi(QString api);
    bool handleQueueCmd(MagicDevice *device, const DRequestPacket &packet);

    void pQueuedCmdStop(MagicDevice *device, const DRequestPacket &packet);
    bool pSendCommand(MagicDevice *device, const DRequestPacket &packet);

    void handleDownloadCmd(MagicDevice *device, const DRequestPacket &packet);

    void _closeAllDevice();

    inline void _sendResMessage(const quint64 id, const QJsonValue resValue);
    inline void _sendResMessage(const DRequestPacket &request, const QJsonValue resValue);
    inline void _sendErrorMessage(const quint64 id, const ErrorType type);
    inline void _sendErrorMessage(const DRequestPacket &request, const ErrorType type);

    void _broadcastForSearchM1();
    QStringList getHostIpList();

private slots:
    virtual void pReceiveMassage_slot(QString id, QJsonObject obj) override;

    void onSearchTimeout_slot();

    /* message from Device */
    void handleReceiveMessage_slot(quint64 id, QString cmd, int res, QJsonValue params);
    void handleCheckDeviceType_slot(quint64 id, QString cmd, int res, QJsonValue params);
    void handleDownloadFinished_slot(quint64 id, bool isOk, QJsonObject errorObj);
    void onUdpReadyRead_slot();
};

#endif // MAGICDEVICEPLUGIN_H
