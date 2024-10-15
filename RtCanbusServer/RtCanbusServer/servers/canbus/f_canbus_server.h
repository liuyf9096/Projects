#ifndef F_CANBUS_SERVER_H
#define F_CANBUS_SERVER_H

#include <QObject>
#include <QCanBus>
#include <QByteArray>
#include <QBitArray>
#include <QMap>

class FCanbusDevice;
class FCanbusServer : public QObject
{
    Q_OBJECT
public:
    static FCanbusServer *GetInstance();

    static QBitArray bytesToBits(QByteArray bytes);
    static QByteArray bitsToBytes(QBitArray bits);
    static void searchDevices();

    void setVerbose(bool en) { m_verbose = en; }
    void setMainAddress(quint16 address) { mMainAddress = address; }
    void addDevice(const QString &dev_id, quint16 address);

signals:
    /* Receive */
    void onReceiveMessage_signal(int address, const QJsonObject &obj, const QByteArray &resValue);

public slots:
    /* Send */
    void sendCanMessage_slot(quint16 canid, quint16 destID, quint16 module, quint16 cmd,
                             const QByteArray &params, const QString &userinfo);

private:
    explicit FCanbusServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(FCanbusServer)

    QCanBusDevice *m_device;
    bool m_verbose;
    quint16 mMainAddress;

    QList<QCanBusFrame> m_sendFrameList;
    QMap<int, FCanbusDevice *> mReceiveHandlerMap;

private slots:
    void onStateChanged_slot(QCanBusDevice::CanBusDeviceState state);
    void processErrors(QCanBusDevice::CanBusError error);
    void processReceivedFrames();
};

#endif // F_CANBUS_SERVER_H
