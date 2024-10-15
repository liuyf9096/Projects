#ifndef MODEULE_BASE_H
#define MODEULE_BASE_H

#include "device/rt_device_manager.h"
#include "sample/rt_sample_manager.h"
#include "messagecenter/f_message_center.h"
#include "sql/f_sql_database_manager.h"

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

class RtDeviceManager;
class DModuleBase : public QObject
{
    Q_OBJECT
    friend class MManagerBase;
public:
    explicit DModuleBase(const QString &mid, const QString &userid = QString(), QObject *parent = nullptr);

    QString mid() { return mModuleId; }
    QString userid() { return mUserId; }

    void setDeviceManager(RtDeviceManager *manager) { dev = manager; }
    RtDeviceManager *dev;

    void sendUIMessage(JPacket &p);
    enum AlarmType { Alarm, Error };
    void sendUIAlarm(AlarmType type, const QString &alarm_id, int code, const QString &message);

    void showMessage(const QJsonObject &obj);
    void logProcess(const QString &process, int state, const QString &statemsg, const QString &message = QString());

    virtual void start();
    virtual void reset();
    virtual void stop();

signals:
    void onDisplayMessage_signal(const QString &mid, const QJsonObject &obj);

protected:
    const QString mModuleId;
    const QString mUserId;

    QString mGroup;

    FSqlDatabase *mLogDb;
    FSqlDatabase *mRecordDb;

private:
    QMap<QString, int> PosCoordMap;
};

#endif // MODEULE_BASE_H
