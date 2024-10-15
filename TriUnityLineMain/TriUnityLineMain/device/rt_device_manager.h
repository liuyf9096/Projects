#ifndef RT_DEVICE_MANAGER_H
#define RT_DEVICE_MANAGER_H

#include "rt_device_base.h"
#include "d_track_1.h"
#include "d_track_2.h"

#include <QObject>
#include <QJsonArray>
#include <QMap>

class FSqlDatabase;
class RtDeviceManager : public QObject
{
    Q_OBJECT
public:
    static RtDeviceManager *GetInstance();

    void configModuleSetup(const QJsonArray &arr);

    /* used for widget */
    void sendAllDeviceReset();
    void sendDeviceReset(const QString &dev_id);

    QList<RtDeviceBase *> deviceList() { return DeviceMap.values(); }
    RtDeviceBase *getDevice(int address);
    RtDeviceBase *getDevice(const QString &dev_id);

    DTrack1 *track1() { return mTrack1; }
    DTrack2 *track2() { return mTrack2; }

signals:
    void close_signal();
    void deviceConnected_signal(const QString &dev_id, bool on);

public slots:
    void close_slot();
    void handleSqlReply_slot(const QString &key, const QJsonArray &arr);

private:
    explicit RtDeviceManager(QObject *parent = nullptr);

    void _init();
    void _configModule();

    FSqlDatabase *m_configDb;

    DTrack1 *mTrack1;
    DTrack2 *mTrack2;

    /* Device Config */
    QMap<QString, RtDeviceBase*> DeviceMap;
    QMap<RtDeviceBase*, QThread*> m_deviceThreadMap;
};

#endif // RT_DEVICE_MANAGER_H
