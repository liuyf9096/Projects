#include "f_common.h"
#include "file/f_file_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDebug>

static const QString AppName = "UnityLineMain";

FCommon *FCommon::GetInstance()
{
    static FCommon *instance = nullptr;
    if (instance == nullptr) {
        instance = new FCommon();
    }
    return instance;
}

FCommon::FCommon(QObject *parent)
    : QObject(parent)
{
    m_configObj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (m_configObj.isEmpty()) {
        qFatal("config.json is empty!");
    }
    m_verbose = false;
    m_stationCount = getConfigValue("general", "station_count").toInt();
    m_existCount = getConfigValue("general", "exit_count").toInt();
}

void FCommon::printSystemInfo()
{
    qDebug().noquote() << "\033[1;32;40m" << QString("[%1] version: %2, Author: liuyufei, release: %3")
                          .arg(FCommon::appName(), FCommon::appVersion(), FCommon::releaseDate())
                       << "\033[0m" << endl;
}

void FCommon::printConfigInfo()
{
    qDebug() << "Config Content:" << m_configObj << endl;
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
    str.append(" ");
    str.append(__TIME__);

    QDateTime datetime = QDateTime::fromString(str, "MMM d yyyy hh:mm:ss");
    if (datetime.isNull()) {
        datetime = QDateTime::fromString(str, "MMM dd yyyy hh:mm:ss");
        if (datetime.isNull()) {
            return str;
        }
    }
    return datetime.toString("yyyy/MM/dd hh:mm:ss");
}

QString FCommon::appPath()
{
    QString path = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    return path;
}

QString FCommon::getPath(const QString &dirName)
{
    QDir dir(appPath());
    if (!dir.exists(dirName)) {
        dir.mkdir(dirName);
    }
    dir.cd(dirName);
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

void FCommon::setVerbose()
{
    m_verbose = true;
    qDebug() << "Mode: verbose";
}

QJsonValue FCommon::getConfigValue(const QString &key)
{
    if (m_configObj.contains(key)) {
        return m_configObj.value(key);
    }
    return QJsonValue();
}

QJsonValue FCommon::getConfigValue(const QString &key1, const QString &key2)
{
    if (m_configObj.contains(key1)) {
        QJsonObject obj1 = m_configObj.value(key1).toObject();
        if (obj1.contains(key2)) {
            return obj1.value(key2);
        }
    }
    return QJsonValue();
}

QJsonValue FCommon::getConfigValue(const QString &key1, const QString &key2, const QString &key3)
{
    if (m_configObj.contains(key1)) {
        QJsonObject obj1 = m_configObj.value(key1).toObject();
        if (obj1.contains(key2)) {
            QJsonObject obj2 = obj1.value(key2).toObject();
            if (obj2.contains(key3)) {
                return obj2.value(key3);
            }
        }
    }
    return QJsonValue();
}

QJsonValue FCommon::getConfigFileValue(const QString &key)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key)) {
        return obj.value(key);
    }
    return QJsonValue();
}

QJsonValue FCommon::getConfigFileValue(const QString &key1, const QString &key2)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key1)) {
        QJsonObject obj1 = obj.value(key1).toObject();
        if (obj1.contains(key2)) {
            return obj1.value(key2);
        }
    }
    return QJsonValue();
}

QJsonValue FCommon::getConfigFileValue(const QString &key1, const QString &key2, const QString &key3)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key1)) {
        QJsonObject obj1 = obj.value(key1).toObject();
        if (obj1.contains(key2)) {
            QJsonObject obj2 = obj1.value(key2).toObject();
            if (obj2.contains(key3)) {
                return obj2.value(key3);
            }
        }
    }
    return QJsonValue();
}

bool FCommon::setConfigFileValue(const QString &key1, const QString &key2, const QJsonValue &value2)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key1)) {
        QJsonObject obj1 = obj.value(key1).toObject();
        if (obj1.contains(key2)) {
            obj1.insert(key2, value2);
            obj.insert(key1, obj1);
            bool ok = FFileManager::writeJsonFileObj(obj, "config", FFileManager::POS_CONFIG);
            return ok;
        }
    }
    return false;
}

bool FCommon::setConfigFileValue(const QString &key1, const QString &key2, const QString &key3, const QJsonValue &value3)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key1)) {
        QJsonObject obj1 = obj.value(key1).toObject();
        if (obj1.contains(key2)) {
            QJsonObject obj2 = obj1.value(key2).toObject();
            if (obj2.contains(key3)) {
                obj2.insert(key3, value3);
                obj1.insert(key2, obj2);
                obj.insert(key1, obj1);
                bool ok = FFileManager::writeJsonFileObj(obj, "config", FFileManager::POS_CONFIG);
                return ok;
            }
        }
    }
    return false;
}

bool FCommon::setConfigFileValue(const QString &key1, const QString &key2, const QString &key3, const QString &key4, const QJsonValue &value4)
{
    QJsonObject obj = FFileManager::readJsonFileObj("config", FFileManager::POS_CONFIG);
    if (obj.contains(key1)) {
        QJsonObject obj1 = obj.value(key1).toObject();
        if (obj1.contains(key2)) {
            QJsonObject obj2 = obj1.value(key2).toObject();
            if (obj2.contains(key3)) {
                QJsonObject obj3 = obj2.value(key3).toObject();
                if (obj3.contains(key4)) {
                    obj3.insert(key4, value4);
                    obj2.insert(key3, obj3);
                    obj1.insert(key2, obj2);
                    obj.insert(key1, obj1);
                    bool ok = FFileManager::writeJsonFileObj(obj, "config", FFileManager::POS_CONFIG);
                    return ok;
                }
            }
        }
    }
    return false;
}
