#ifndef MODULE_BASE_H
#define MODULE_BASE_H

#include "device/rt_device_manager.h"
#include "sample/rt_sample_manager.h"
#include "messagecenter/f_message_center.h"
#include "exception/exception_center.h"
#include "sql/f_sql_database_manager.h"
#include "f_common.h"

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QJsonObject>
#include <QDebug>

class FSqlDatabase;
class RtDeviceManager;
class DModuleBase : public QObject
{
    Q_OBJECT
    friend class MManagerBase;
public:
    explicit DModuleBase(const QString &mid, const QString &userid = QString(), QObject *parent = nullptr);

    QString mid() { return mModuleId; }
    QString userid() { return mUserId; }
    void setLogLabel(const QString &label) { mLabel = label; }

    void setDeviceManager(RtDeviceManager *manager) { dev = manager; }
    RtDeviceManager *dev;

    void sendUIMessage(JPacket &p);

    virtual void start();
    virtual void reset();
    virtual void stop();

protected:
    const QString mModuleId;
    const QString mUserId;
    QString mLabel;

    FSqlDatabase *mLogDb;
    FSqlDatabase *mRecordDb;

    void logProcess(const QString &process, int state, const QString &statemsg, const QString &message = QString());
    void recordModuleSampleState(const QString &module, const QString &table, QSharedPointer<RtSample> sample = nullptr);
    void recordModuleSampleState(const QString &module, bool isCapped);
    void recordModuleSlideState(const QString &module, const QString &table, QSharedPointer<RtSlide> slide = nullptr);
    void recordSlotSlideState(int pos, QSharedPointer<RtSlide> slide);
};

#endif // MODULE_BASE_H
