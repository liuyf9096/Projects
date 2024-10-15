#ifndef RT_DEVICE_MANAGER_H
#define RT_DEVICE_MANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QMap>

class FSqlDatabase;
class RtDevice;
class RtDeviceManager : public QObject
{
    Q_OBJECT
public:
    static RtDeviceManager *GetInstance();

    void configModuleSetup(const QJsonArray &arr);

    RtDevice *getDevice(int address);
    RtDevice *getDevice(const QString &dev_id);

signals:
    void close_signal();
    void deviceConnected_signal(const QString &dev_id, bool on);

private:
    explicit RtDeviceManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(RtDeviceManager)

    RtDevice *addDevice(const QString &dev_id, const QJsonObject &obj);
    void setCommandMap(RtDevice *device, const QJsonArray &arr);

    FSqlDatabase *m_db;

    QList<RtDevice *> m_deviceList;
    QMap<QString, RtDevice *> m_deviceMap;  // <dev_id, rtdevice>
};

#endif // RT_DEVICE_MANAGER_H
