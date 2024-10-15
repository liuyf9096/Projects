#include "rt_device_manager.h"
#include "f_common.h"

#include "rt_device.h"
#include "canbus/f_canbus_server.h"
#include "sql/f_sql_database_manager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
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
    qDebug(" ");
    qDebug("--------- CONFIG ---------");

    m_db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (m_db != nullptr) {
        QJsonArray module_arr = m_db->selectRecord("device");
        configModuleSetup(module_arr);
        m_db->closeDatabase();
    } else {
        qFatal("database config is Missing.");
    }
}

void RtDeviceManager::configModuleSetup(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString dev_id = obj.value("dev_id").toString();
        int enable = obj.value("enable").toInt();
        if (enable == 0) {
            continue;
        }

        auto device = addDevice(dev_id, obj);
        int function_enable = obj.value("function_enable").toInt();
        int sensor_enable = obj.value("sensor_enable").toInt();
        int floater_sensor_enable = obj.value("floater_sensor_enable").toInt();

        if (function_enable > 0) {
            QJsonArray function_arr = m_db->selectRecord(QString("%1_function").arg(dev_id));
            if (function_arr.count() > 0) {
                qDebug().noquote() << QString("[%1] set Function list:").arg(dev_id);
                setCommandMap(device, function_arr);
                qDebug(" ");
            }
        }
        if (sensor_enable > 0) {
            QJsonArray sensor_arr = m_db->selectRecord(QString("%1_sensors").arg(dev_id));
            if (sensor_arr.count() > 0) {
                qDebug().noquote() << QString("[%1] set Sensors list:").arg(dev_id);
                device->setSensors(sensor_arr);
                qDebug(" ");
            }
        }
        if (floater_sensor_enable > 0) {
            QJsonArray floater_sensor_arr = m_db->selectRecord(QString("%1_floater_sensors").arg(dev_id));
            if (floater_sensor_arr.count() > 0) {
                qDebug().noquote() << QString("[%1] set Floater Sensors list:").arg(dev_id);
                device->setFloaterSensors(floater_sensor_arr);
                qDebug(" ");
            }
        }
    }
}

RtDevice *RtDeviceManager::addDevice(const QString &dev_id, const QJsonObject &obj)
{
    quint16 address = obj.value("address").toInt();
    QString username = obj.value("username").toString();
    QString type = obj.value("type").toString(dev_id);
    QString description = obj.value("decription").toString();
    int canbusMsgInterval = obj.value("canbus_message_interval").toInt(20);
    int checkSensorInterval = obj.value("check_sensor_interval").toInt(200);

    RtDevice *device = new RtDevice(dev_id, address, type, this);
    device->setUsername(username);
    device->setSendMsgInterval(canbusMsgInterval);
    device->setCheckSensorInterval(checkSensorInterval);

    /* device <-> canbus */
    auto canbus = FCanbusServer::GetInstance();
    canbus->addDevice(dev_id, address);
    connect(device, &RtDevice::sendCanMessage_signal,
            canbus, &FCanbusServer::sendCanMessage_slot);
    connect(canbus, &FCanbusServer::onReceiveMessage_signal,
            device, &RtDevice::handleCanbusMessage_slot);

    m_deviceList.append(device);
    m_deviceMap.insert(dev_id, device);

    qInfo().noquote() << QString("[Manager] ADD (%1)[%2] %3").arg(dev_id, username, description);

    return device;
}

void RtDeviceManager::setCommandMap(RtDevice *device, const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject functionObj = arr.at(i).toObject();

        QString function = functionObj.value("function").toString();
        int func_id = functionObj.value("func_id").toInt(0);
        QString type = functionObj.value("type").toString();
        int timeout = functionObj.value("timeout").toInt(0);
        QString description = functionObj.value("description").toString();

        if (type == "ComboAction") {
            device->setComboAction(function, func_id, timeout, description);
        }
    }
}

RtDevice *RtDeviceManager::getDevice(int address)
{
    foreach (auto device, m_deviceList) {
        if (device->getAddress() == address) {
            return device;
        }
    }
    return nullptr;
}

RtDevice *RtDeviceManager::getDevice(const QString &dev_id)
{
    foreach (auto device, m_deviceList) {
        if (device->deviceID() == dev_id) {
            return device;
        }
    }
    return nullptr;
}


