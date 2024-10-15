#include "f_serialport.h"

#include <QJsonArray>
#include <QDebug>

FSerialPort::FSerialPort(QObject *parent) : QObject(parent)
{

}

QJsonValue FSerialPort::getAvailablePortInfo()
{
    QJsonArray array;

    foreach (const auto &portInfo, QSerialPortInfo::availablePorts()) {
        QJsonObject obj;
        obj.insert("portName", portInfo.portName());
        obj.insert("description", portInfo.description());
        array.append(obj);
    }
    return QJsonValue(array);
}

QStringList FSerialPort::getAvailablePortList()
{
    QStringList list;
    foreach (const auto &portInfo, QSerialPortInfo::availablePorts()) {
        QString str = QString("%1 (%2)").arg(portInfo.portName()).arg(portInfo.description());
        list.append(str);
    }
    return list;
}

