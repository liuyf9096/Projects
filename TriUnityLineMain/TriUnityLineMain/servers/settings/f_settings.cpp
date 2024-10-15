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
    QDir dir(FCommon::configPath());

    QString appName = FCommon::appName().toLower();
    appName.append(".ini");

    if (!dir.exists(appName)) {
        qFatal("Error! File app.ini missing.");
    }

    m_settings = new QSettings(dir.absoluteFilePath(appName), QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");

    qInfo() << "app config path:" << m_settings->fileName();

#if 0
    defaultValueInit();
#endif
}

void FSettings::defaultValueInit()
{
    m_settings->setValue("isAgingTestMode", false);
    m_settings->setValue("ExitCount", 2);
    m_settings->setValue("StationCount", 3);
    m_settings->setValue("SmearOnly", false);
    m_settings->setValue("SmearAll", false);
    m_settings->setValue("TestAll", false);
    m_settings->setValue("Connect_IPU", false);
    m_settings->setValue("Connect_UI", false);

    m_settings->beginGroup("Aging_Record");
    m_settings->setValue("count", 0);
    m_settings->setValue("closet1_count", 0);
    m_settings->setValue("closet2_count", 0);
    m_settings->endGroup();

    m_settings->beginGroup("Record");
    m_settings->setValue("RecordLogEnable", false);
    m_settings->setValue("RecordLogSqlEnable", false);
    m_settings->setValue("RecordLogToText", false);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_Canbus");
    m_settings->setValue("address", "127.0.0.1");
    m_settings->setValue("port", 9096);
    m_settings->setValue("reconnect_interval", 1000);
    m_settings->endGroup();

    m_settings->beginGroup("WebSocket_IPU");
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
    m_settings->endGroup();

    /* Station */
    m_settings->beginGroup("S1");
    m_settings->setValue("devid", "BF800");
    m_settings->setValue("address", "0.0.0.0");
    m_settings->setValue("isUnited", true);
    m_settings->endGroup();

    m_settings->beginGroup("S2");
    m_settings->setValue("devid", "BF800");
    m_settings->setValue("address", "0.0.0.0");
    m_settings->setValue("isUnited", false);
    m_settings->endGroup();

    m_settings->beginGroup("S3");
    m_settings->setValue("devid", "BD2000");
    m_settings->setValue("address", "0.0.0.0");
    m_settings->setValue("isUnited", true);
    m_settings->endGroup();

    m_settings->beginGroup("Program_Default");
    m_settings->setValue("program", "5-classify");
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
bool FSettings::isAgingTestMode()
{
    return m_settings->value("isAgingTestMode").toBool();
}

int FSettings::getExitCount()
{
    return m_settings->value("ExitCount").toInt();
}

int FSettings::getStationCount()
{
    return m_settings->value("StationCount").toInt();
}

bool FSettings::isSmearOnly()
{
    return m_settings->value("SmearOnly").toBool();
}

bool FSettings::isSmearAll()
{
    return m_settings->value("SmearAll").toBool();
}

bool FSettings::isTestAll()
{
    return m_settings->value("TestAll").toBool();
}

bool FSettings::isConnectIPU()
{
    return m_settings->value("Connect_IPU").toBool();
}

bool FSettings::isConnectUI()
{
    return m_settings->value("Connect_UI").toBool();
}

quint64 FSettings::agingCount()
{
    return m_settings->value("Aging_Record/count").toUInt();
}

quint64 FSettings::closet1Count()
{
    return m_settings->value("Aging_Record/closet1_count").toUInt();
}

quint64 FSettings::closet2Count()
{
    return m_settings->value("Aging_Record/closet2_count").toUInt();
}

void FSettings::saveAgingCount(quint64 count)
{
    m_settings->setValue("Aging_Record/count", count);
}

void FSettings::saveCloset1Count(quint64 count)
{
    m_settings->setValue("Aging_Record/closet1_count", count);
}

void FSettings::saveCloset2Count(quint64 count)
{
    m_settings->setValue("Aging_Record/closet2_count", count);
}

QString FSettings::canbus_server_addr()
{
    return m_settings->value("WebSocket_Canbus/address").toString();
}

quint16 FSettings::canbus_server_port()
{
    return m_settings->value("WebSocket_Canbus/port").toInt();
}

QString FSettings::ipu_server_addr()
{
    return m_settings->value("WebSocket_IPU/address").toString();
}

quint16 FSettings::ipu_server_port()
{
    return m_settings->value("WebSocket_IPU/port").toInt();
}

QString FSettings::ui_server_addr()
{
    return m_settings->value("WebSocket_UI/address").toString();
}

quint16 FSettings::ui_server_port()
{
    return m_settings->value("WebSocket_UI/port").toInt();
}

quint16 FSettings::ws_server_port()
{
    return m_settings->value("Websocket_Server/port").toInt();
}

bool FSettings::isRecordLogEnable()
{
    return m_settings->value("Record/RecordLogEnable").toBool();
}

bool FSettings::isRecordLogToText()
{
    return m_settings->value("Record/RecordLogToText").toBool();
}

QString FSettings::defaultProgram()
{
    return m_settings->value("Program_Default/program").toString();
}

QString FSettings::station1_deviceid()
{
    return m_settings->value("S1/devid").toString();
}

QString FSettings::station1_address()
{
    return m_settings->value("S1/address").toString();
}

bool FSettings::isStation1_united()
{
    return m_settings->value("S1/isUnited").toBool();
}

QString FSettings::station2_deviceid()
{
    return m_settings->value("S2/devid").toString();
}

QString FSettings::station2_address()
{
    return m_settings->value("S2/address").toString();
}

bool FSettings::isStation2_united()
{
    return m_settings->value("S2/isUnited").toBool();
}

QString FSettings::station3_deviceid()
{
    return m_settings->value("S3/devid").toString();
}

QString FSettings::station3_address()
{
    return m_settings->value("S3/address").toString();
}

bool FSettings::isStation3_united()
{
    return m_settings->value("S3/isUnited").toBool();
}

void FSettings::setStation1_Device_Id(const QString &devid)
{
    m_settings->setValue("S1/devid", devid);
}

void FSettings::setStation1_Device_Address(const QString &address)
{
    m_settings->setValue("S1/address", address);
}

void FSettings::setStation1_United(bool isUnited)
{
    m_settings->setValue("S1/isUnited", isUnited);
}

void FSettings::setStation2_Device_Id(const QString &devid)
{
    m_settings->setValue("S2/devid", devid);
}

void FSettings::setStation2_Device_Address(const QString &address)
{
    m_settings->setValue("S2/address", address);
}

void FSettings::setStation2_United(bool isUnited)
{
    m_settings->setValue("S2/isUnited", isUnited);
}

void FSettings::setStation3_Device_Id(const QString &devid)
{
    m_settings->setValue("S3/devid", devid);
}

void FSettings::setStation3_Device_Address(const QString &address)
{
    m_settings->setValue("S3/address", address);
}

void FSettings::setStation3_United(bool isUnited)
{
    m_settings->setValue("S3/isUnited", isUnited);
}

void FSettings::setLogPath(const QString &path)
{
    m_settings->setValue("log_path", path);
}
