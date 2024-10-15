#include "f_websocket_manager.h"
#include <QDebug>

static const QString G_ClientName = "BD2000";

FWebSocketManager *FWebSocketManager::GetInstance()
{
    static FWebSocketManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new FWebSocketManager();
    }
    return instance;
}

FWebSocketManager::FWebSocketManager(QObject *parent) : QObject(parent)
{
}

FWebSocket *FWebSocketManager::addSocket(const QString &userName)
{
    if (!m_socketMap.contains(userName)) {
        auto socket = new FWebSocket(this);
        socket->setClientName(G_ClientName);
        m_socketMap.insert(userName, socket);
        return socket;
    } else {
        qDebug() << "Socket Map has already a name, called:" << userName;
        return m_socketMap.value(userName);
    }
}

FWebSocket *FWebSocketManager::getSocket(const QString &userName)
{
    if (!m_socketMap.contains(userName)) {
        qFatal("Can NOT find Socket!");
    }
    return m_socketMap.value(userName);
}


