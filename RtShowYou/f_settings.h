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

    quint16 getListenPort();
    quint16 getSendPort();

private:
    explicit FSettings(QObject *parent = nullptr);
    Q_DISABLE_COPY(FSettings)

    void defaultValueInit();

    QSettings *m_settings;
};

#endif // F_SETTINGS_H
