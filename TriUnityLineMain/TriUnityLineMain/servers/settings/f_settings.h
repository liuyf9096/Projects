#ifndef F_SETTINGS_H
#define F_SETTINGS_H

#include <QObject>
#include <QVariant>
#include <QSettings>
#include <QMap>

class FSettings : public QObject
{
    Q_OBJECT
public:
    static FSettings *GetInstance();

    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key);

    bool isAgingTestMode();
    int getExitCount();
    int getStationCount();
    bool isSmearOnly();
    bool isSmearAll();
    bool isTestAll();
    bool isConnectIPU();
    bool isConnectUI();

    quint64 agingCount();
    quint64 closet1Count();
    quint64 closet2Count();
    void saveAgingCount(quint64 count);
    void saveCloset1Count(quint64 count);
    void saveCloset2Count(quint64 count);

    QString canbus_server_addr();
    quint16 canbus_server_port();
    QString ipu_server_addr();
    quint16 ipu_server_port();
    QString ui_server_addr();
    quint16 ui_server_port();

    quint16 ws_server_port();

    bool isRecordLogEnable();
    bool isRecordLogToText();

    QString defaultProgram();

    QString station1_deviceid();
    QString station1_address();
    bool isStation1_united();

    QString station2_deviceid();
    QString station2_address();
    bool isStation2_united();

    QString station3_deviceid();
    QString station3_address();
    bool isStation3_united();

    void setStation1_Device_Id(const QString &devid);
    void setStation1_Device_Address(const QString &address);
    void setStation1_United(bool isUnited);

    void setStation2_Device_Id(const QString &devid);
    void setStation2_Device_Address(const QString &address);
    void setStation2_United(bool isUnited);

    void setStation3_Device_Id(const QString &devid);
    void setStation3_Device_Address(const QString &address);
    void setStation3_United(bool isUnited);

    void setLogPath(const QString &path);

private:
    explicit FSettings(QObject *parent = nullptr);
    Q_DISABLE_COPY(FSettings)

    void defaultValueInit();

    QSettings *m_settings;
};

#endif // F_SETTINGS_H
