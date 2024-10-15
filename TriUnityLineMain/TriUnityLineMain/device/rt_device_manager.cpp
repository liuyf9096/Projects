#include "rt_device_manager.h"
#include "f_common.h"
#include "sql/f_sql_database_manager.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

RtDeviceManager *RtDeviceManager::GetInstance()
{
    static RtDeviceManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new RtDeviceManager();
    }
    return instance;
}

RtDeviceManager::RtDeviceManager(QObject *parent) : QObject(parent)
{
    _init();
    _configModule();
}

void RtDeviceManager::_init()
{
    qDebug(" ");
    qDebug("--------- CONFIG ---------");
    mTrack1 = new DTrack1("track_1", this);
    mTrack2 = new DTrack2("track_2", this);

    DeviceMap.insert("track_1", mTrack1);
    DeviceMap.insert("track_2", mTrack2);
}

void RtDeviceManager::_configModule()
{
    m_configDb = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (m_configDb) {
        auto arr = m_configDb->selectRecord("device");
        configModuleSetup(arr);
        m_configDb->closeDatabase();
    }
}

void RtDeviceManager::configModuleSetup(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString dev_id = obj.value("dev_id").toString();

        if (DeviceMap.contains(dev_id)) {
            auto device = DeviceMap.value(dev_id);

            quint16 address = obj.value("address").toInt();
            device->setAddress(address);

            QString username = obj.value("username").toString();
            device->setUsername(username);

            int enable = obj.value("enable").toInt();
            if (enable == 0) {
                device->setEnable(false);
                continue;
            } else {
                device->setEnable(true);
            }

            qInfo().noquote() << QString("[%1](%2) Set address:0x%3")
                                 .arg(device->deviceID(), username).arg(address, 2, 16, QChar('0'));

            QString function_str = QString("%1_function").arg(dev_id);
            QJsonArray function_arr = m_configDb->selectRecord(function_str);
            device->SetFuncTimeoutMap(function_arr);

            QString sensor_str = QString("%1_sensors").arg(dev_id);
            QJsonArray sensor_arr = m_configDb->selectRecord(sensor_str);
            device->SetSensorPortMap(sensor_arr);
            qDebug(" ");
        }
    }
}

void RtDeviceManager::close_slot()
{
    qDebug() << __FUNCTION__;

    emit close_signal();

    DeviceMap.clear();
}

void RtDeviceManager::handleSqlReply_slot(const QString &key, const QJsonArray &arr)
{
    if (key == "query_modules") {
        configModuleSetup(arr);
    }
}

RtDeviceBase *RtDeviceManager::getDevice(int address)
{
    for (auto device : qAsConst(DeviceMap)) {
        if (device->address() == address) {
            return device;
        }
    }
    return nullptr;
}

RtDeviceBase *RtDeviceManager::getDevice(const QString &dev_id)
{
    if (DeviceMap.contains(dev_id)) {
        return DeviceMap.value(dev_id);
    }
    return nullptr;
}

void RtDeviceManager::sendAllDeviceReset()
{
    for (auto device : qAsConst(DeviceMap)) {
        if (device) {
            device->cmd_Reset();
        }
    }
}

void RtDeviceManager::sendDeviceReset(const QString &dev_id)
{
    auto device = getDevice(dev_id);
    if (device) {
        device->cmd_Reset();
    }
}

