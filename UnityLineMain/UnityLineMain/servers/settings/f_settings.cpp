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
    m_settings->setValue("AgingTestMode", false);
    m_settings->setValue("ExitCount", 2);
    m_settings->setValue("StationCount", 3);
    m_settings->setValue("Connect_DMU", false);
    m_settings->setValue("Connect_UI", false);

    m_settings->beginGroup("Test");
    m_settings->setValue("SmearOnly", false);
    m_settings->setValue("SmearAll", false);
    m_settings->setValue("TestAll", false);
    m_settings->endGroup();

    m_settings->beginGroup("Aging_Record");
    m_settings->setValue("count", 0);
    m_settings->setValue("closet1_count", 0);
    m_settings->setValue("closet2_count", 0);
    m_settings->endGroup();

    m_settings->beginGroup("Log");
    m_settings->setValue("LogServer_enable", false);
    m_settings->setValue("save_log_to_text", true);
    m_settings->setValue("save_log_to_sql", false);
    m_settings->setValue("max_logfile_count", 10);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_Canbus");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 9096);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_DMU");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 10090);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_UI");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 10010);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->endGroup();

    /* Client: S1,S2,S3 */
    m_settings->beginGroup("Websocket_Server");
    m_settings->setValue("port", 10086);
    m_settings->setValue("Send_UDP", true);
    m_settings->endGroup();

    /* Station */
    m_settings->beginGroup("S1");
    m_settings->setValue("devid", "BF800");
    m_settings->setValue("address", "0.0.0.0");
    m_settings->setValue("type", "bloodtest");
    m_settings->setValue("isUnited", true);
    m_settings->endGroup();

    m_settings->beginGroup("S2");
    m_settings->setValue("devid", "BD2000");
    m_settings->setValue("address", "0.0.0.0");
    m_settings->setValue("type", "smear");
    m_settings->setValue("isUnited", true);
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

