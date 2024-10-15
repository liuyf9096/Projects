#ifndef R_UDP_SERVER_H
#define R_UDP_SERVER_H

#include <QObject>
#include <QFile>
#include <QJsonObject>

class QTimer;
class QUdpSocket;
class RUdpServer : public QObject
{
    Q_OBJECT
public:
    explicit RUdpServer(QObject *parent = nullptr);

    QString getIPAddress();
    QStringList getIPAddressList();

    void sendUdpMessage(quint16 port, const QString &message);

private:
    QString m_ip;
    QStringList m_iplist;
    quint16 m_bindPort;
    quint16 m_sendPort;
    QString m_name;
    QStringList m_keywordList;

    int m_checkInterval;
    QTimer *m_timer;
    QUdpSocket *m_udpSocket;

    QFile m_file;
    QJsonObject m_fileObj;

    void _init();
    void _bindUdp();
    QString getHostName();
    bool writeNewName(const QString &name);

    QByteArray readCongfigFile(const QString &filename);
    void parseConfigFileData(const QByteArray &data);

private slots:
    void onReadDatagram_slot();
    void onTimer_slot();
};

#endif // R_UDP_SERVER_H
