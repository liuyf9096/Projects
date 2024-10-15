#include "rt_device_manager.h"
#include "f_common.h"
#include "sql/f_sql_database_manager.h"
#include "exception/exception_center.h"

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
    qDebug("[RtDeviceManager] init..");

    _init();
    _configModule();

    qDebug("[RtDeviceManager] init..OK.");
    qDebug() << " ";
}

void RtDeviceManager::_init()
{
    qDebug("--------- CONFIG ---------");

    mTrack = new DTrack("track", this);
    mSample = new DSample("sample", this);
    mSmear = new DSmear("smear", this);
    mStain = new DStain("stain", this);
    mPrint = new DPrint("print", this);

    DeviceMap.insert("track", mTrack);
    DeviceMap.insert("sample", mSample);
    DeviceMap.insert("smear", mSmear);
    DeviceMap.insert("stain", mStain);
    DeviceMap.insert("print", mPrint);

    mSample->setBoardNum(1);
    BoardNumMap.insert("sample", 1);
    mStain->setBoardNum(2);
    BoardNumMap.insert("stain", 2);
    mSmear->setBoardNum(3);
    BoardNumMap.insert("smear", 3);
    mTrack->setBoardNum(4);
    BoardNumMap.insert("track", 4);
    mPrint->setBoardNum(5);
    BoardNumMap.insert("print", 5);
}

void RtDeviceManager::_configModule()
{
    m_configDb = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (m_configDb) {
        auto arr = m_configDb->selectRecord("device");
        configModuleSetup(arr);
//        m_configDb->closeDatabase();
    }
}

void RtDeviceManager::configModuleSetup(const QJsonArray &arr)
{
    auto mException = ExceptionCenter::GetInstance();

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

            connect(device, &RtDeviceBase::onFunctionFailed_signal,
                    mException, &ExceptionCenter::onFunctionFailed_slot);
            connect(device, &RtDeviceBase::onFunctionTimeout_signal,
                    mException, &ExceptionCenter::onFunctionTimeout_slot);
            connect(device, &RtDeviceBase::onFunctionFinished_signal,
                    mException, &ExceptionCenter::onFunctionFinished_slot);

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

RtDeviceBase *RtDeviceManager::getDevice(int address)
{
    foreach (auto device, DeviceMap) {
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
    foreach (auto device, DeviceMap) {
        if (device->isEnable()) {
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

