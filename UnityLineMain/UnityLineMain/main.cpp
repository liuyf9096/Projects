#include "f_common.h"
#include "rt_init.h"
#include "views/mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    FCommon::printSystemInfo();

    /* servers */
    if (argc > 1) {
        if (qstrcmp(argv[1], "-verbose") == 0 || qstrcmp(argv[1], "-v") == 0) {
            FCommon::GetInstance()->setVerbose();
        }
    }
    servers_init();

#if 0
    /* languages */
    QTranslator translator;
    if (translator.load("://chinese.qm")) {
        app.installTranslator(&translator);
    }
#endif

#ifdef Q_OS_WIN
    MainWindow w;
    w.show();
#endif

    return app.exec();
}


