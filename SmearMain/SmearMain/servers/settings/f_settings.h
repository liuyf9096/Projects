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

    /* General */
    bool isUnited();
    bool isDebugMode();

    /* Stain */
    int stain_Check_interval();

    /* websocket */
    QString canbus_server_addr();
    quint16 canbus_server_port();

    QString unity_server_addr();
    quint16 unity_server_port();
    bool unity_server_autodetect();
    QString unity_server_detectKey();

    QString reader_server_addr();
    quint16 reader_server_port();
    bool reader_server_autodetect();
    QString reader_server_detectKey();

    void setUnity_server_addr(const QString &addr);
    void setUnity_server_port(quint16 port);
    void setReader_server_addr(const QString &addr);
    void setReader_server_port(quint16 port);

    /* websocket server (ui) */
    quint16 ws_server_port();

    /* Log */
    uint logindex();
    uint MaxLogFileCount();
    bool isLogServerEnable();
    bool isSaveLogToText();

private:
    explicit FSettings(QObject *parent = nullptr);
    Q_DISABLE_COPY(FSettings)

    void defaultValueInit();

    QSettings *m_settings;
};

#endif // F_SETTINGS_H
