#include <QApplication>
#include <QSharedMemory>
#include <QTranslator>

#include "DobotLinkMain.h"
#include "DelayToQuit/DelayToQuit.h"

#define SINGLEAPPLICATION

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName(QStringLiteral("Dobot"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("dobot.cc"));
    QCoreApplication::setApplicationName(QStringLiteral("DobotLink"));
    QApplication::setApplicationName(QStringLiteral("DobotLink"));
    QApplication::setOrganizationName(QStringLiteral("Dobot"));
    QApplication::setOrganizationDomain(QStringLiteral("dobot.cc"));
    QApplication::setWindowIcon(QIcon(QStringLiteral("://images/DobotLinkIcon.ico")));
    QApplication::setApplicationDisplayName(QStringLiteral("DobotLink"));

#ifdef SINGLEAPPLICATION
    /* make sure only one application is running */
    QSharedMemory shared("DobotLink_Running");
    if (shared.attach()) {
        DelayToQuit dQuit;
        dQuit.start();
        return app.exec();
    }
    shared.create(1);
#endif 

    QTranslator translator;
    if (translator.load("://dobotlink.qm")) {
        app.installTranslator(&translator);
    }

    DobotLinkMain mainWindow;
//    mainWindow.show();

    return app.exec();
}
