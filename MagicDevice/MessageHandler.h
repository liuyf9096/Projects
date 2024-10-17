#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QJsonObject>

#include "DPacket.h"

class QStringList;
class QTimer;
class QSerialPort;
class QUdpSocket;
class MessageHandler : public QObject
{
    Q_OBJECT

public:
    enum ProtocolType {DEVICE_MAGIC_PROTOCOL, DEVICE_M1_PROTOCOL};

    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler();

    void setProtocolType(ProtocolType type);
    bool isConnected();

    void setPortName(QString portName);
    QString getPortName();

    void setHostIpAddress(QString ip);
    void setUdpIpAddress(QString ip);
    QString getUdpIpAddress();
    void setDeviceUdpPort(quint16 port);

    Q_INVOKABLE void connectDevice(quint64 id = 0);
    Q_INVOKABLE void disconnectDevice(quint64 id = 0);

    Q_INVOKABLE void addSendingList(DPacket packet);
    Q_INVOKABLE void setTimeoutValue(int ms);

signals:
    void onConnectStatus_signal(int code, quint64 id = 0);
    void sendMessages_signal(QJsonObject message);          // 收到的信息
    void onErrorOccurred_signal(int code, quint64 id = 0);  // 其他错误信息(CommunicationError)

private:
    enum HandleState {IdleState, SendingCmdState, WaitingCmdAckState} m_currentState;

    ProtocolType m_ProtocolType;

    bool m_isConnected;
    QList<DPacket> m_sendingList;

    /* SerialPort / UDP */
    bool isSerialConnection;

    /* SerialPort */
    QSerialPort *m_serialPort;

    /* UDP socket */
    QUdpSocket *m_udpSocket;
    QString m_localhostIp;
    QString m_deviceUdpIp;
    quint16 m_deviceUdpPort;

    /* Packet */
    DPacket currentPacket;
    DPacket receivePacket;

    /* Buffer: in case get less packet */
    QByteArray m_rxBuffer;

    /* Timer */
    QTimer *m_PeriodTimer;
    QTimer *m_TimeOutTimer;
    QTimer *m_KeepAliveTimer;

    /* time over */
    int m_TimeoverMs;
    bool m_isTimeOver;

    /* resend count */
    int m_resendCount;

    quint64 m_connectCmdID;

    void _SerialPortInit();
    void _UdpSocketInit();
    void _TimerInit();
    bool _bindLocalHostIp();

    void _SendData(QByteArray data);
    void _HandleNewData(QByteArray newdata);

private slots:
    void _PeriodicTask_slot();

    void onSerialReadyRead_slot();
    void onReadDatagram_slot();
};

#endif // MESSAGEHANDLER_H
