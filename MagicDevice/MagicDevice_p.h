#ifndef MAGICDEVICE_P_H
#define MAGICDEVICE_P_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QList>

#include "ActionTimerManager.h"

enum ConnectStatus {UNCONNECTED, CONNECTED, OCCUPIED, UNKNOWN};

class QThread;
class MessageHandler;
class MessagePacket;
class MagicDevice;
class MagicDevicePrivate : public QObject
{
    Q_OBJECT
public:
    MagicDevicePrivate(MagicDevice *parent);
    virtual ~MagicDevicePrivate();

    static QMap<int, QString> DeviceTypeMap;
    static QMap<int, QString> initDeviceTypeStringMap();
    static QString checkIsIpAddress(QString ip);

    quint16 m_wsport;
    int m_deviceType;

    float poseX, poseY, poseZ, poseR;
    ConnectStatus m_status;

    typedef QList<MessagePacket> DPacketList;
    QMap<quint64, MessagePacket> m_RequestMap;
    QMap<quint64, DPacketList> m_PacketListMap;

    MessageHandler *m_MessageHandler;
    QThread *m_thread;

    ActionTimerManager *m_actionTimerManager;

private:
    MagicDevice * const q_ptr;
    Q_DISABLE_COPY(MagicDevicePrivate)
    Q_DECLARE_PUBLIC(MagicDevice)

private slots:
    void checkCurrentIndex_slot();
    void handleConnectResult_slot(int code, quint64 id);
    void handleErrorResult_slot(int code, quint64 id);
    void handleCommonResult_slot(QJsonObject message);
    void handleActionTimeout_slot(quint64 id, ActionTimerManager::FinishType type);
};

#endif // MAGICDEVICE_P_H
