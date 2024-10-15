#ifndef RT_SAMPLE_MANAGER_H
#define RT_SAMPLE_MANAGER_H

#include "rt_sample.h"
#include "messagecenter/f_message_center.h"

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QSharedPointer>

class FSqlDatabase;
class RtSampleManager : public QObject
{
    Q_OBJECT
public:
    static RtSampleManager *GetInstance();

    QSharedPointer<RtSample> NewSample();
    QSharedPointer<RtSample> getSample(const QString &sid);

    void removeSampleOne(const QString &sid);
    void removeSampleOne(QSharedPointer<RtSample> sample);
    void removeSampleAll();

    void sql_record(const QString &sid, const QJsonObject &setObj = QJsonObject());
    void sql_recordStatus(const QString &sid, SampleStatus status, int value);

    bool requestSampleOrderUid(QSharedPointer<RtSample> sample);
    bool requestSampleTestUid(const QString &dev_id, QSharedPointer<RtSample> sample);

    void setSampleReviewTask(const JPacket &request);

    void sendOperationInfo(const QString &error_id, const QString &sid = QString());
    void clearQueryMessage();

    void sendSamplePath(QSharedPointer<RtSample> sample, const QString &node_id);

public slots:
    void handleSampleOrder_slot(const JPacket &result, const JPacket &request);
    void handleSampleState_slot(const QString &client_id, const JPacket &note);

private:
    explicit RtSampleManager(QObject *parent = nullptr);

    quint64 m_sid;

    QMap<QString, QSharedPointer<RtSample>> m_sampleMap;
    QMap<QString, QSharedPointer<RtSample>> m_orderUIDMap;
    QMap<QString, QSharedPointer<RtSample>> m_testUIDMap;
    QMap<quint64, QString> m_queryOrderUIDMap;
    QMap<quint64, QString> m_queryTestUIDMap;

    FSqlDatabase *mLogDb;   
};

#endif // RT_SAMPLE_MANAGER_H
