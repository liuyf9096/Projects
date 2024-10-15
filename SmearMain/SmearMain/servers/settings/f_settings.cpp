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
    m_settings->setValue("united", false);
    m_settings->setValue("debug", false);
    m_settings->setValue("readerEnable", false);

    m_settings->beginGroup("Stain");
    m_settings->setValue("Check_interval", 45);
    m_settings->setValue("isAutoFillPool", false);
    m_settings->endGroup();

    m_settings->beginGroup("Log");
    m_settings->setValue("index", 0);
    m_settings->setValue("LogServer_enable", false);
    m_settings->setValue("save_log_to_text", false);
    m_settings->setValue("save_log_to_sql", false);
    m_settings->setValue("max_logfile_count", 100);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_Canbus");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 9096);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_Unity");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 10096);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->setValue("isAutoDetect", true);
    m_settings->setValue("autodetect_key", "United");
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_Reader");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 10090);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->setValue("isAutoDetect", true);
    m_settings->setValue("autodetect_key", "Reader");
    m_settings->endGroup();

    m_settings->beginGroup("Websocket_Server");
    m_settings->setValue("port", 10096);
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

bool FSettings::isUnited()
{
    return m_settings->value("united").toBool();
}

bool FSettings::isDebugMode()
{
    return m_settings->value("debug").toBool();
}

int FSettings::stain_Check_interval()
{
    return m_settings->value("Stain/Check_interval").toInt();
}

QString FSettings::canbus_server_addr()
{
    return m_settings->value("WebSocket_Canbus/address").toString();
}

quint16 FSettings::canbus_server_port()
{
    return m_settings->value("WebSocket_Canbus/port").toInt();
}

QString FSettings::unity_server_addr()
{
    return m_settings->value("WebSocket_Unity/address").toString();
}

quint16 FSettings::unity_server_port()
{
    return m_settings->value("WebSocket_Unity/port").toInt();
}

bool FSettings::unity_server_autodetect()
{
    return m_settings->value("WebSocket_Unity/isAutoDetect").toBool();
}

QString FSettings::unity_server_detectKey()
{
    return m_settings->value("WebSocket_Unity/autodetect_key").toString();
}

QString FSettings::reader_server_addr()
{
    return m_settings->value("WebSocket_Reader/address").toString();
}

quint16 FSettings::reader_server_port()
{
    return m_settings->value("WebSocket_Reader/port").toInt();
}

bool FSettings::reader_server_autodetect()
{
    return m_settings->value("WebSocket_Reader/isAutoDetect").toBool();
}

QString FSettings::reader_server_detectKey()
{
    return m_settings->value("WebSocket_Reader/autodetect_key").toString();
}

void FSettings::setUnity_server_addr(const QString &addr)
{
    m_settings->setValue("WebSocket_Unity/address", addr);
}

void FSettings::setUnity_server_port(quint16 port)
{
    m_settings->setValue("WebSocket_Unity/port", port);
}

void FSettings::setReader_server_addr(const QString &addr)
{
    m_settings->setValue("WebSocket_Reader/address", addr);
}

void FSettings::setReader_server_port(quint16 port)
{
    m_settings->setValue("WebSocket_Reader/port", port);
}

quint16 FSettings::ws_server_port()
{
    return m_settings->value("Websocket_Server/port").toInt();
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

