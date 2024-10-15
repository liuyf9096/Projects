#include <QCoreApplication>

#include "r_udp_server.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "ShowMe" << "v1.1.1" << "liuyufei@reetoo";

    RUdpServer s;
    return a.exec();
}

