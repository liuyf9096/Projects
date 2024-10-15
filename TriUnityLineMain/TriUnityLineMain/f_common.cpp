#include "f_common.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QLocale>
#include <QDebug>

const QString AppName = "TriUnityLineMain";

FCommon *FCommon::GetInstance()
{
    static FCommon *instance = nullptr;
    if (instance == nullptr) {
        instance = new FCommon();
    }
    return instance;
}

FCommon::FCommon(QObject *parent) : QObject(parent)
{
    m_isDebugMode = false;
}

void FCommon::printSystemInfo()
{
    qDebug().noquote() << QString("[%1] version: %2 release: %3")
                          .arg(FCommon::appName(), FCommon::appVersion(), FCommon::releaseDate())
                       << endl;
}

QString FCommon::appName()
{
    return AppName;
}

QString FCommon::appVersion()
{
    return QString("%1.%2.%3").arg(Version_major).arg(Version_minor).arg(Version_patch);
}

QString FCommon::releaseDate()
{
    QString str;
    str.append(__DATE__);
    str.append(__TIME__);

    QDateTime datetime = QLocale(QLocale::English).toDateTime(str, "MMM d yyyyhh:mm:ss");
    if (datetime.isNull()) {
        return str;
    }
    return datetime.toString("yyyy/MM/dd hh:mm:ss");
}

QString FCommon::appPath()
{
    QString path = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    return path;
}

QString FCommon::configPath()
{
    QDir dir(appPath());
    if (dir.exists("config")) {
        dir.cd("config");
    } else {
        qFatal("config files is missing.");
    }
    return dir.absolutePath();
}

QString FCommon::logPath()
{
    QDir dir(appPath());
    if (!dir.exists("log")) {
        dir.mkdir("log");
    }
    dir.cd("log");
    return dir.absolutePath();
}

QString FCommon::checkIpAddress(const QString &address)
{
    QRegExp rx("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
    if (rx.indexIn(address) > -1) {
        QString ip = rx.cap();
        return ip;
    }
    return QString();
}

void FCommon::setDebugMode(bool debug)
{
    m_isDebugMode = debug;
    qDebug() << "DebugMode:" << debug;
}


