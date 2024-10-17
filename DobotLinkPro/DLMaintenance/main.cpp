#include "MainWindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load("://dlmaintenance.qm")) {
        app.installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return app.exec();
}
