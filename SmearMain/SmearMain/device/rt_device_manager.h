#ifndef RT_DEVICE_MANAGER_H
#define RT_DEVICE_MANAGER_H

#include "rt_device_base.h"
#include "d_track.h"
#include "d_sample.h"
#include "d_smear.h"
#include "d_stain.h"
#include "d_print.h"

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
    int getBoardNum(const QString &dev_id) { return BoardNumMap.value(dev_id); }

    DTrack *track() { return mTrack; }
    DSample *sample() { return mSample; }
    DSmear *smear() { return mSmear; }
    DStain *stain() { return mStain; }
    DPrint *print() { return mPrint; }

signals:
    void close_signal();
    void deviceConnected_signal(const QString &dev_id, bool on);

public slots:
    void close_slot();

private:
    explicit RtDeviceManager(QObject *parent = nullptr);

    void _init();
    void _configModule();

    FSqlDatabase *m_configDb;

    DTrack *mTrack;
    DSample *mSample;
    DSmear *mSmear;
    DStain *mStain;
    DPrint *mPrint;

    /* Device Config */
    QMap<QString, RtDeviceBase*> DeviceMap;
    QMap<QString, int> BoardNumMap;
    QMap<RtDeviceBase*, QThread*> m_deviceThreadMap;
};

#endif // RT_DEVICE_MANAGER_H
