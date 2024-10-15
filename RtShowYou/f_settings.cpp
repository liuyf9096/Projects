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
    QDir dir(FCommon::appPath());

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
    m_settings->beginGroup("udp_port");
    m_settings->setValue("listen", 12445);
    m_settings->setValue("send", 12345);
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
quint16 FSettings::getListenPort()
{
    quint16 port = m_settings->value("udp_port/listen").toInt();
    return port;
}

quint16 FSettings::getSendPort()
{
    quint16 port = m_settings->value("udp_port/send").toInt();
    return port;
}

