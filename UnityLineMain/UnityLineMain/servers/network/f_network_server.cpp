#include "f_network_server.h"

#include <QJsonDocument>
#include <QUrlQuery>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QDebug>

FNetworkServer *FNetworkServer::GetInstance()
{
    static FNetworkServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new FNetworkServer();
    }
    return instance;
}

FNetworkServer::FNetworkServer(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

FNetworkServer::~FNetworkServer()
{
    qDebug() << "RtNetworkServer destroyed.";
}

void FNetworkServer::showIPAddress()
{
    auto list = QNetworkInterface::allInterfaces();
    for (const auto &interface : qAsConst(list))
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
            for (const auto &entry : qAsConst(entryList)) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    qInfo().noquote() << QString("[NetworkServer] local address: %1").arg(entry.ip().toString());
                }
            }
        }
    }
}

QStringList FNetworkServer::getIPAddress()
{
    QStringList iplist;
    auto list = QNetworkInterface::allInterfaces();
    for (const auto &interface : qAsConst(list))
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
            for (const auto &entry : qAsConst(entryList)) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    iplist.append(entry.ip().toString());
                    qInfo().noquote() << QString("[NetworkServer] local address: %1").arg(entry.ip().toString());
                }
            }
        }
    }
    return iplist;
}
