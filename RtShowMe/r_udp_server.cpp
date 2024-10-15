#include "r_udp_server.h"

#include <QCoreApplication>
#include <QUrlQuery>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUdpSocket>
#include <QHostInfo>
#include <QTimer>
#include <QDebug>

RUdpServer::RUdpServer(QObject *parent) : QObject(parent)
{
    m_checkInterval = 1000;

    _init();
    _bindUdp();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &RUdpServer::onTimer_slot);
    m_timer->start(m_checkInterval);
}

void RUdpServer::_init()
{
    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &RUdpServer::onReadDatagram_slot);
}

void RUdpServer::_bindUdp()
{
    QByteArray data = readCongfigFile("config.json");
    parseConfigFileData(data);

    bool ok = m_udpSocket->bind(m_bindPort);
    if (ok) {
        qInfo().noquote() << QString("bind port:%1 success.").arg(m_bindPort);
    } else {
        qFatal("Error! udp bind Fail.");
    }
}

QString RUdpServer::getIPAddress()
{
    QString ip;

    foreach (auto interface, QNetworkInterface::allInterfaces())
    {
        if (!interface.isValid()) {
            continue;
        }

        QNetworkInterface::InterfaceFlags flags = interface.flags();
        if (flags.testFlag(QNetworkInterface::IsRunning)
                && !flags.testFlag(QNetworkInterface::IsLoopBack))
        {
            if (interface.humanReadableName().contains("VMware")
                    || interface.humanReadableName().contains("Loopback")
                    || interface.humanReadableName().contains("VirtualBox"))
            {
                continue;
            }

            QList<QNetworkAddressEntry> entryList = interface.addressEntries();
            foreach (auto entry, entryList) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ip = entry.ip().toString();
                }
            }
        }
    }

    return ip;
}

QStringList RUdpServer::getIPAddressList()
{
    QStringList list;

    foreach (auto interface, QNetworkInterface::allInterfaces())
    {
        if (!interface.isValid()) {
            continue;
        }

        QNetworkInterface::InterfaceFlags flags = interface.flags();
        if (flags.testFlag(QNetworkInterface::IsRunning)
                && !flags.testFlag(QNetworkInterface::IsLoopBack))
        {
            if (interface.humanReadableName().contains("VMware")
                    || interface.humanReadableName().contains("Loopback")
                    || interface.humanReadableName().contains("VirtualBox"))
            {
                continue;
            }

            QList<QNetworkAddressEntry> entryList = interface.addressEntries();
            foreach (auto entry, entryList) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    list.append(entry.ip().toString());
                }
            }
        }
    }

    return list;
}

QByteArray RUdpServer::readCongfigFile(const QString &filename)
{
    QString path = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    QDir dir(path);
    qDebug() << "config.json path:" << dir.absoluteFilePath(filename);

    if (dir.exists(filename)) {
        m_file.setFileName(dir.absoluteFilePath(filename));
        bool ok = m_file.open(QIODevice::ReadOnly | QIODevice::Text);
        if(ok == true) {
            QByteArray content = m_file.readAll();
            m_file.close();
            return content;
        } else {
            qFatal("Error! config.json can NOT open.");
        }
    } else {
        qFatal("Error! config.json is missing.");
    }
    return QByteArray();
}

void RUdpServer::parseConfigFileData(const QByteArray &data)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "Device Setup File: Json Parse Error" << jsonError.errorString();
        return;
    }

    if (doc.isObject()) {
        m_fileObj = doc.object();
        if (m_fileObj.contains("check_interval")) {
            m_checkInterval = m_fileObj.value("check_interval").toInt();
            qDebug() << "check interval:" << m_checkInterval;
        }
        if (m_fileObj.contains("key")) {
            QJsonValue keyValue = m_fileObj.value("key");
            m_keywordList.clear();
            if (keyValue.isArray()) {
                QJsonArray arr = keyValue.toArray();
                for (int i = 0; i < arr.count(); ++i) {
                    m_keywordList.append(arr.at(i).toString());
                }
            } else {
                m_keywordList.append(keyValue.toString());
            }
        } else {
            qWarning() << "config key word is missing";
        }
        if (m_fileObj.contains("port")) {
            QJsonObject portObj = m_fileObj.value("port").toObject();
            m_bindPort = portObj.value("listen").toInt();
            m_sendPort = portObj.value("send").toInt();
        } else {
            qFatal("Error! consfig port is missing.");
        }
        if (m_fileObj.contains("name")) {
            m_name = m_fileObj.value("name").toString();
        }
        qDebug() << "config:" << m_fileObj;
    }
}

void RUdpServer::sendUdpMessage(quint16 port, const QString &message)
{
    qDebug().noquote() << QString("<< [UDP:%1]").arg(port) << message;
    m_udpSocket->writeDatagram(message.toUtf8(), QHostAddress::Broadcast, port);
}

void RUdpServer::onReadDatagram_slot()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QString str(datagram.data());
        qDebug()<< ">> [UDP]" << str;

        if (m_keywordList.contains(str)) {
            sendUdpMessage(m_sendPort, QString("%1,%2,%3").arg(QHostInfo::localHostName(), m_iplist.join("; "), m_name));
        } else if (str.startsWith("ChangeDecription")) {
            QStringList list = str.split(",");
            if (list.count() > 2) {
                QString ip = list.at(1);
                if (m_iplist.contains(ip)) {
                    m_name = list.at(2);
                    bool ok = writeNewName(m_name);
                    if (ok) {
                        sendUdpMessage(m_sendPort, QString("%1,%2,%3").arg(QHostInfo::localHostName(), m_ip, m_name));
                    }
                }
            }
        }
    }
}

bool RUdpServer::writeNewName(const QString &name)
{
    bool ok = m_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    if(ok == true) {
        m_fileObj.insert("name", name);

        QJsonDocument doc(m_fileObj);
        QByteArray data = doc.toJson();

        m_file.write(data);
        m_file.close();
        return true;
    }
    return false;
}

void RUdpServer::onTimer_slot()
{
#if 0
    QString ip = getIPAddress();
    if (ip != m_ip) {
        m_ip = ip;
        qInfo().noquote() << QString("[ShowMe] ip address: %1").arg(ip);
    }
#else
    QStringList list = getIPAddressList();
    if (list != m_iplist) {
        m_iplist = list;
        qInfo() << "[ShowMe] ip address:" << list.join(", ");
    }
#endif
}
