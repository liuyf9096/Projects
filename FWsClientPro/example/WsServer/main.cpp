#include <QCoreApplication>

#include "f_websocket_server.h"
#include "mytask.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FWebSocketServer::GetInstance()->listenPort(10086);
    MyTask task;

    return a.exec();
}
