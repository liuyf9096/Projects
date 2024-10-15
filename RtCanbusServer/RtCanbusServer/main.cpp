#include "f_common.h"
#include "rt_init.h"

#ifdef Q_OS_WIN
#include "views/mainwindow.h"
#endif

#include "tasks/upgrader.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(FCommon::appName());
    QCoreApplication::setApplicationVersion(FCommon::appVersion());

    FCommon::printSystemInfo();

    QCommandLineParser parser;
    parser.setApplicationDescription("Canbus Communication Server");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("directory", "directory");

    QCommandLineOption runOption(QStringList() << "r" << "run", "Run function.", "func_id");
    parser.addOption(runOption);

    QCommandLineOption verboseOption(QStringList() << "p" << "verbose", "Print more information.");
    parser.addOption(verboseOption);

    QCommandLineOption upgradeOption(QStringList() << "u" << "upgrade", "Upgrade board software.",
                                     "filename", "./upgrade/programs.json");
    parser.addOption(upgradeOption);

    QCommandLineOption upgradeDefaultOption(QStringList() << "upgrade_default_path", "Upgrade board software.");
    parser.addOption(upgradeDefaultOption);

    QCommandLineOption boardOption(QStringList() << "b" << "board", "Specify the board.", "board");
    parser.addOption(boardOption);

    parser.process(app);

    if (parser.isSet("verbose")) {
        FCommon::GetInstance()->setVerbose();
        qDebug() << "Mode: Verbose";
    }

    /* servers */
    servers_init();

#ifdef Q_OS_WIN
    MainWindow w;
    w.show();
#endif

    if (parser.isSet("upgrade")) {
        Upgrader::GetInstance()->upgradeBoards(parser.value("upgrade"));
    } else if (parser.isSet("upgrade_default_path")) {
        Upgrader::GetInstance()->upgradeBoards("programs.json");
    }

    return app.exec();
}

