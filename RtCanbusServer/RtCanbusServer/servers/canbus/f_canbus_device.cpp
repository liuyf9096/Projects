#include "f_canbus_device.h"
#include "f_canbus_protocol.h"
#include <QDebug>

FCanbusDevice::FCanbusDevice(quint16 destid, QObject *parent)
    : QObject(parent)
    , address(destid)
{

}

QByteArray FCanbusDevice::append(const QCanBusFrame &frame)
{
    QByteArray result;
    FCanidObj canid(frame.frameId());

    if (canid.srcid() != address || canid.count() < 1) {
        qWarning() << "framehandler error, address:" << address << "count:" << canid.count();
        return result;
    }

    if (canid.index() == 1) {
        m_list.clear();
    }

    m_list.append(frame);

    if (canid.index() >= canid.count()) {
        for (int i = 0; i < m_list.count(); ++i) {
            result.append(m_list.at(i).payload());
        }
    }
    return result;
}

