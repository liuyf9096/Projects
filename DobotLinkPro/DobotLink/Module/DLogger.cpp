#include "DLogger.h"

#include <QTextStream>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QMutexLocker>
#include <QTextCodec>
#include <QDebug>

DLogger *DLogger::getInstance()
{
    static DLogger *instance = nullptr;
    if (instance == nullptr) {
        instance = new DLogger();
    }
    return instance;
}

DLogger::DLogger(QObject *parent) : QObject(parent)
{
    isLogging = false;
    logFile = new QFile();

    msgHead << "Debug   " << "Warning " << "Critical" << "Fatal   " << "Info    ";

    /* log-file-path setting */
#ifdef Q_OS_WIN
    logPath.setPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    logPath.cd("AppData");
    logPath.cd("Roaming");
#elif defined (Q_OS_MAC)
    logPath.setPath(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
#elif defined (Q_OS_LINUX)
    logPath.setPath("/var");
    logPath.cd("log");
#endif

    if (!logPath.exists("DobotLink")) {
        logPath.mkdir("DobotLink");
    }
    if (logPath.cd("DobotLink")) {
        QString dataTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QString fileName = QString("DL_%1.log").arg(dataTime);
        logFile->setFileName(logPath.absoluteFilePath(fileName));
    }
}

void DLogger::destroyLog_slot()
{
    stopLogging();

    if (logFile->isOpen()) {
        logFile->close();
        qDebug() << "Close log file";
    }
    delete logFile;
}

void DLogger::startLogging()
{
    if (isLogging) {
        return;
    }

    if (logFile->isOpen()) {
        isLogging = true;
        qInstallMessageHandler(myMessageOutput);
        qDebug() << "Start logging at path:" << logFile->fileName();
    } else if (logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        isLogging = true;
        qInstallMessageHandler(myMessageOutput);
        qDebug() << "Start logging at path:" << logFile->fileName();
    } else {
        qWarning() << "Open log File Failed:" << logFile->fileName();
    }
}

void DLogger::stopLogging()
{
    qDebug() << "stop logging.";
    qDebug() << "-- THE END --";

    isLogging = false;
    qInstallMessageHandler(nullptr);

    if (logFile->isOpen()) {
        logFile->close();
    }
}

void DLogger::openLogFile()
{
    QFileInfo fileInfo(logFile->fileName());
    if (fileInfo.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    } else {
        qDebug() << "log file is Not exist.";
    }
}

bool DLogger::openLogDir()
{
    bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(logPath.absolutePath()));
    return ok;
}


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    DLogger *logger = DLogger::getInstance();

    QString msg;
    if (type == QtWarningMsg) {
        msg.append("[!WARNING]");
    }
    msg.append(message);

    emit logger->logMessage_signal(msg);

    if (!logger->logFile->isOpen()) {
        return;
    }

    QMutexLocker locker(&logger->mMutex);

    QTextStream textStream(logger->logFile);

    QString messageText;
    QString currentDateTime;

#ifdef QT_NO_DEBUG
    /* simple print */
    Q_UNUSED(context)

    currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    messageText = QString("%1|%2\n").arg(currentDateTime).arg(msg);
#else
    /* details print */
    currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    messageText = QString("%1 | %6 | %2:%3, %4 | %5\n")
            .arg(logger->msgHead.at(type))
            .arg(context.file)
            .arg(context.line)
            .arg(context.function)
            .arg(message)
            .arg(currentDateTime);
#endif

    textStream << messageText.toUtf8();
    logger->logFile->flush();
}

#if 0
    QString appPath = QCoreApplication::applicationDirPath(); //app 路径
    QString otherPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation); //系统路径
    QString otherPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    SetFileAttributes((LPCWSTR)(filePath.unicode()), FILE_ATTRIBUTE_HIDDEN);    //#include "windows.h" 隐藏文件
#endif
