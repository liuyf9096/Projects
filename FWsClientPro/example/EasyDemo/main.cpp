#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.connectServer("127.0.0.1", 10001);
    w.show();
    return a.exec();
}
