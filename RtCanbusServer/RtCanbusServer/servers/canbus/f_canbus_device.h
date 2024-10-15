#ifndef F_CANBUS_DEVICE_H
#define F_CANBUS_DEVICE_H

#include <QObject>
#include <QJsonObject>
#include <QList>
#include <QCanBus>

class FCanbusDevice : public QObject
{
    Q_OBJECT
public:
    explicit FCanbusDevice(quint16 destid, QObject *parent = nullptr);

    const quint16 address;

    QByteArray append(const QCanBusFrame &frame);

private:
    QList<QCanBusFrame> m_list;
};

#endif // F_CANBUS_DEVICE_H
