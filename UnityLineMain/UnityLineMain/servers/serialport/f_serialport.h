#ifndef F_SERIALPORT_H
#define F_SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QJsonObject>
#include <QMap>

class FSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit FSerialPort(QObject *parent = nullptr);

    static QJsonValue getAvailablePortInfo();
    static QStringList getAvailablePortList();
};

#endif // F_SERIALPORT_H
