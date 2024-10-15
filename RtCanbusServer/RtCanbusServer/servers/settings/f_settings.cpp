#include "f_settings.h"
#include "f_common.h"

#include <QDir>
#include <QDebug>

FSettings *FSettings::GetInstance()
{
    static FSettings *instance = nullptr;
    if (instance == nullptr) {
        instance = new FSettings();
    }
    return instance;
}

FSettings::FSettings(QObject *parent) : QObject(parent)
{
    QDir dir(FCommon::getPath("config"));

    QString inifile = FCommon::appName().toLower();
    inifile.append(".ini");

    if (!dir.exists(inifile)) {
        QString str = QString("Error! File %1 is missing.").arg(inifile);
        qFatal(str.toLatin1().data());
    }

    m_settings = new QSettings(dir.absoluteFilePath(inifile), QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");

    qInfo() << "app config path:" << m_settings->fileName();

#if 0
    defaultValueInit();
#endif
}

void FSettings::defaultValueInit()
{
    m_settings->beginGroup("Log");
    m_settings->setValue("index", 0);
    m_settings->setValue("LogServer_enable", false);
    m_settings->setValue("save_log_to_text", false);
    m_settings->setValue("save_log_to_sql", false);
    m_settings->setValue("max_logfile_count", 100);
    m_settings->endGroup();

    m_settings->beginGroup("Websocket_Server");
    m_settings->setValue("port", 9096);
    m_settings->endGroup();

    m_settings->beginGroup("Canbus");
    m_settings->setValue("verbose", false);
    m_settings->endGroup();

}

void FSettings::setValue(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

QVariant FSettings::getValue(const QString &key)
{
    return m_settings->value(key);
}

/* function */
quint16 FSettings::getWebSocketPort()
{
    quint16 port = m_settings->value("Websocket_Server/port").toInt();
    return port;
}

bool FSettings::getCanbusVerbose()
{
    return m_settings->value("Canbus/verbose").toBool();
}

uint FSettings::logindex()
{
    uint index = m_settings->value("Log/index").toUInt() + 1;
    m_settings->setValue("Log/index", index);
    return index;
}

bool FSettings::isLogServerEnable()
{
    return m_settings->value("Log/LogServer_enable").toBool();
}

bool FSettings::isSaveLogToText()
{
    return m_settings->value("Log/save_log_to_text").toBool();
}

uint FSettings::MaxLogFileCount()
{
    return m_settings->value("Log/max_logfile_count").toUInt();
}

