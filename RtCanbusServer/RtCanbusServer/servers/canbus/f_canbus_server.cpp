#include "f_canbus_server.h"
#include "f_canbus_protocol.h"
#include "f_canbus_device.h"

#include <stdlib.h>
#include <QDebug>

FCanbusServer *FCanbusServer::GetInstance()
{
    static FCanbusServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new FCanbusServer();
    }
    return instance;
}

FCanbusServer::FCanbusServer(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_LINUX
    system("ifconfig can0 down");
    system("ip link set can0 type can bitrate 500000");
    system("ifconfig can0 up");
#endif

    m_verbose = false;

    QString errorString;
    m_device = QCanBus::instance()->createDevice(QStringLiteral("socketcan"), QStringLiteral("can0"), &errorString);
    if (m_device == nullptr) {
        qWarning() << "[RtCanbusServer] device create fail." << errorString;
    } else {
        bool ok = m_device->connectDevice();
        if (ok) {
            connect(m_device, &QCanBusDevice::stateChanged,
                    this, &FCanbusServer::onStateChanged_slot);
            connect(m_device, &QCanBusDevice::errorOccurred,
                    this, &FCanbusServer::processErrors);
            connect(m_device, &QCanBusDevice::framesReceived,
                    this, &FCanbusServer::processReceivedFrames);
        } else {
            qDebug() << "Canbus can NOT conneted.";
        }
    }
}

void FCanbusServer::searchDevices()
{
    QString errorString;
    const QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(QStringLiteral("socketcan"), &errorString);
    if (!errorString.isEmpty()) {
        qDebug() << errorString;
    }

    if (!devices.isEmpty()) {
        QStringList list;
        foreach (auto info, devices) {
            list.append(info.name());
        }
        qDebug() << "[CanBusServer]" << "canbus devices:" << list;
    } else {
        qDebug() << "[CanBusServer]" << "canbus devices count: 0";
    }
}

void FCanbusServer::addDevice(const QString &dev_id, quint16 address)
{
    if (dev_id == "main") {
        mMainAddress = address;
        qInfo().noquote() << QString("Canbus Main Board address: 0x%1").arg(mMainAddress, 0, 16);
    } else if (mReceiveHandlerMap.contains(address) == false) {
        auto device = new FCanbusDevice(address, this);
        mReceiveHandlerMap.insert(address, device);
    }
}

void FCanbusServer::sendCanMessage_slot(quint16 canid, quint16 destID, quint16 module, quint16 cmd,
                                        const QByteArray &params, const QString &userinfo)
{
    if (m_device == nullptr) {
        qWarning() << "can device is invalid.";
        return;
    }

    QByteArray data = FCanBusPayload::encode(canid, module, cmd, params);
    FCanidObj canidObj(mMainAddress, destID, data.length());

    for (int i = 0; i < canidObj.count(); ++i) {
        QCanBusFrame frame(canidObj.canidFromIndex(i + 1), data.mid(i*8, 8));
        bool ok = m_device->writeFrame(frame);
		
		FCanidObj canidObj_t(frame.frameId());
        if (ok) {
            if (m_verbose == true) {
                qDebug() << "< " << canidObj_t.toString() << frame.payload().toHex(' ');
            }
        } else {
            qWarning() << canidObj_t.toString() << frame.payload().toHex(' ') << "send fail.";
        }
    }

#ifdef CAN_VERBOSE
    qDebug().noquote() << QString("<< [CAN send] canid:%1 (%2) [cmd:%3]").arg(canid).arg(userinfo).arg(cmd, 0, 16);
#endif
}

void FCanbusServer::onStateChanged_slot(QCanBusDevice::CanBusDeviceState state)
{
    QString canName = m_device->property("username").toString();
    switch (state) {
    case QCanBusDevice::UnconnectedState:
        qDebug() << "[CanBusServer]" << canName << "Unconnected.";
        break;
    case QCanBusDevice::ConnectingState:
        qDebug() << "[CanBusServer]" << canName << "Connecting.";
        break;
    case QCanBusDevice::ConnectedState:
        qDebug() << "[CanBusServer]" << canName << "Connected.";
        break;
    case QCanBusDevice::ClosingState:
        qDebug() << "[CanBusServer]" << canName << "Closing.";
        break;
    }
}

void FCanbusServer::processErrors(QCanBusDevice::CanBusError error)
{
    switch (error) {
    case QCanBusDevice::NoError: break;
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        qWarning() << "[CanBusServer] errors" << error << m_device->errorString();
        break;
    }
}

void FCanbusServer::processReceivedFrames()
{
    while (m_device->framesAvailable())
    {
        const QCanBusFrame frame = m_device->readFrame();
        if (frame.frameType() == QCanBusFrame::DataFrame)
        {
            FCanidObj canidObj(frame.frameId());

            if (m_verbose == true) {
                qDebug() << "> " << canidObj.toString() << frame.payload().toHex(' ');
            }

            if (canidObj.destid() == mMainAddress && mReceiveHandlerMap.contains(canidObj.srcid()))
            {
                FCanbusDevice *device = mReceiveHandlerMap.value(canidObj.srcid());

                QByteArray result = device->append(frame);
                if (result.isEmpty() == false)
                {
                    QByteArray res_value;
                    QJsonObject obj = FCanBusPayload::decode(result, res_value);
                    if (obj.isEmpty() == false)
                    {
                        if (m_verbose == true)
                        {
                            int id = obj.value("id").toInt();
                            int state = obj.value("state").toInt();
                            if (state == 1) {
                                qDebug().noquote() << QString(">> [ACK id:%1] addr:%2").arg(id).arg(device->address);
                            } else {
                                qDebug().noquote() << QString(">> [receive id:%1] addr:%2").arg(id).arg(device->address)
                                                   << obj << res_value.toHex(' ');
                            }
                        }

                        emit onReceiveMessage_signal(device->address, obj, res_value);
                    }
                }
            } else {
                qWarning() << "Unknown Device address:" << canidObj.srcid();
            }
        } else {
            qWarning() << "Not DataFrame" << frame.error() << m_device->interpretErrorFrame(frame);
        }
    }
}

QBitArray FCanbusServer::bytesToBits(QByteArray bytes)
{
    QBitArray bits(bytes.count() * 8);

    for(int i = 0; i < bytes.count(); ++i) {
        for(int b = 0; b < 8; ++b) {
            bits.setBit(i * 8 + b, bytes.at(i) & (1 << b));
        }
    }
    return bits;
}

QByteArray FCanbusServer::bitsToBytes(QBitArray bits)
{
    QByteArray bytes;
    bytes.resize(bits.count()/8);

    for(int b = 0; b < bits.count(); ++b) {
        bytes[b / 8] = ( bytes.at(b / 8) | ((bits[b] ? 1 : 0) << (b % 8)));
    }
    return bytes;
}
