#ifndef FWEBSOCKETMANAGER_H
#define FWEBSOCKETMANAGER_H

#include "f_websocket.h"

#include <QObject>
#include <QMap>

class FWebSocketManager : public QObject
{
    Q_OBJECT
public:
    static FWebSocketManager *GetInstance();

    FWebSocket *addSocket(const QString &userName = "default");
    FWebSocket *getSocket(const QString &userName = "default");

private:
    explicit FWebSocketManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(FWebSocketManager)

    QMap<QString, FWebSocket *> m_socketMap;
};

#endif // FWEBSOCKETMANAGER_H
