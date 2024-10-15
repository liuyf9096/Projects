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
    void removeSampleAll();

    void sql_record(const QString &sid, const QJsonObject &setObj = QJsonObject());
    void sql_recordStatus(const QString &sid, SampleStatus status, int value);

    void requestSampleOrderUid(QSharedPointer<RtSample> sample);
    void requestSampleTestUid(const QString &dev_id, QSharedPointer<RtSample> sample);

public slots:
    void handleSampleOrder_slot(const JPacket &result, const JPacket &request);
    void handleSampleState_slot(const QString &, const JPacket &note);

private:
    explicit RtSampleManager(QObject *parent = nullptr);

    quint64 m_sid;
    QString defaultProgram;
    bool isSmearOnly;

    QMap<QString, QSharedPointer<RtSample>> m_sampleMap;
    QMap<QString, QSharedPointer<RtSample>> m_orderUIDMap;
    QMap<QString, QSharedPointer<RtSample>> m_testUIDMap;
    QMap<quint64, QString> m_queryOrderUIDMap;
    QMap<quint64, QString> m_queryTestUIDMap;

    FSqlDatabase *mLogDb;
};

#endif // RT_SAMPLE_MANAGER_H
