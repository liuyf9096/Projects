#include "rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"
#include "servers/settings/f_settings.h"

#include <QStringList>
#include <QDebug>

RtSampleManager *RtSampleManager::GetInstance()
{
    static RtSampleManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new RtSampleManager();
    }
    return instance;
}

RtSampleManager::RtSampleManager(QObject *parent)
    : QObject(parent)
    , m_sid(1)
{
    mLogDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    defaultProgram = FSettings::GetInstance()->defaultProgram();
    isSmearOnly = FSettings::GetInstance()->isSmearOnly();

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onClientsMessageNotification_signal,
            this, &RtSampleManager::handleSampleState_slot);
    connect(center, &FMessageCenter::onIPUMessageResultPacket_signal,
            this, &RtSampleManager::handleSampleOrder_slot);
}

QSharedPointer<RtSample> RtSampleManager::NewSample()
{
    QString sampleid = QString("S%1").arg(m_sid++);
    auto sample = QSharedPointer<RtSample>(new RtSample(sampleid));
    sample->setBloodType("WholeBlood");

    QStringList list;
    if (isSmearOnly == true) {
        sample->setState(SampleStatus::Send_To_Test);
        sample->setState(SampleStatus::Test_Finished);
        sample->setState(SampleStatus::Review_Sample);
        sample->setState(SampleStatus::Need_Smear);
        list << "smear";
    } else {
        list << defaultProgram;
    }

    sample->setProgramList(list);

    m_sampleMap.insert(sampleid, sample);

    return sample;
}

QSharedPointer<RtSample> RtSampleManager::getSample(const QString &sid)
{
    if (m_sampleMap.contains(sid)) {
        return m_sampleMap.value(sid);
    } else {
        qWarning() << "can NOT find Sample sid:" << sid;
    }
    return nullptr;
}

void RtSampleManager::removeSampleOne(const QString &sid)
{
    if (m_sampleMap.contains(sid)) {
        auto sample = m_sampleMap.take(sid);

        QString order_uid = sample->order_uid();
        if (m_orderUIDMap.contains(order_uid)) {
            m_orderUIDMap.remove(order_uid);
        }

        QString test_uid = sample->test_uid();
        if (m_testUIDMap.contains(test_uid)) {
            m_testUIDMap.remove(test_uid);
        }
        qInfo() << "remove sample, id:" << sid;
    }
}

void RtSampleManager::removeSampleAll()
{
    m_sampleMap.clear();
    m_orderUIDMap.clear();
    m_testUIDMap.clear();
    qDebug() << "delete All samples.";
}

void RtSampleManager::sql_record(const QString &sid, const QJsonObject &setObj)
{
    QJsonObject obj;
    obj.insert("sid", sid);

    if (mLogDb) {
        if (setObj.isEmpty()) {
            mLogDb->insertRecord("samples", obj);
        } else {
            mLogDb->updateRecord("samples", setObj, obj);
        }
    }
}

void RtSampleManager::sql_recordStatus(const QString &sid, SampleStatus status, int value)
{
    if (RtSample::StatusMap.contains(status)) {
        QJsonObject obj;
        obj.insert(RtSample::StatusMap.value(status), value);
        sql_record(sid, obj);
    }
}

void RtSampleManager::requestSampleOrderUid(QSharedPointer<RtSample> sample)
{
    bool isconnect = FMessageCenter::GetInstance()->isIPUConnected();
    if (isconnect == false) {
        sample->setDefaultOrderInfo();
        m_orderUIDMap.insert(sample->order_uid(), sample);
        m_testUIDMap.insert(sample->test_uid(), sample);
        qDebug() << "Default Sample order_uid:" << sample->sid() << "test_uid:"<< sample->test_uid();
    } else {
        JPacket p(PacketType::Request);
        p.device = "LabXpert";
        p.module = "Order";
        p.api = "AssignOrderUID";

        QJsonObject obj;
        obj.insert("rack_num", sample->rack_id());
        obj.insert("tube_num", sample->rack_pos() + 1);
        obj.insert("barcode", sample->barcode());
        p.paramsValue = obj;

        quint64 id = FMessageCenter::GetInstance()->sendIPUMessage(p);
        m_queryOrderUIDMap.insert(id, sample->sid());
    }
}

void RtSampleManager::requestSampleTestUid(const QString &dev_id, QSharedPointer<RtSample> sample)
{
    bool isconnect = FMessageCenter::GetInstance()->isIPUConnected();
    if (isconnect == true) {
        JPacket p(PacketType::Request);
        p.device = "LabXpert";
        p.module = "Order";
        p.api = "AssignTestUID";

        QJsonObject obj;
        obj.insert("order_uid", sample->order_uid());
        obj.insert("device_id", dev_id);
        p.paramsValue = obj;

        quint64 id = FMessageCenter::GetInstance()->sendIPUMessage(p);
        m_queryTestUIDMap.insert(id, sample->sid());
    }
}

void RtSampleManager::handleSampleOrder_slot(const JPacket &result, const JPacket &request)
{
    if (request.api == "AssignOrderUID") {
        if (m_queryOrderUIDMap.contains(result.id)) {
            QString sid = m_queryOrderUIDMap.take(result.id);
            auto sample = m_sampleMap.value(sid);
            if (sample) {
                QJsonObject obj = result.resValue.toObject();
                sample->setOrderInfo(obj);
                if (sample->order_uid().isEmpty() == false) {
                    m_orderUIDMap.insert(sample->order_uid(), sample);
                }
                requestSampleTestUid("S1", sample);
            }
        }
    } else if (request.api == "AssignTestUID") {
        if (m_queryTestUIDMap.contains(result.id)) {
            QString sid = m_queryTestUIDMap.take(result.id);
            auto sample = m_sampleMap.value(sid);
            if (sample) {
                QJsonObject obj = result.resValue.toObject();
                if (obj.contains("test_uid")) {
                    QString test_uid = obj.value("test_uid").toString();
                    if (test_uid.isEmpty()) {
                        qWarning() << "test_uid is Empty.";
                    } else {
                        sample->setTest_uid(test_uid);
                        m_testUIDMap.insert(test_uid, sample);
                    }
                } else {
                    qWarning() << "test_uid is MISSING.";
                }
            }
        }
    }
}

void RtSampleManager::handleSampleState_slot(const QString &/*client_id*/, const JPacket &note)
{
    if (note.api == "Sample") {
        auto obj = note.paramsValue.toObject();
        if (obj.contains("test_uid")) {
            QString test_uid = obj.value("test_uid").toString();
            QString status = obj.value("status").toString();
            if (m_testUIDMap.contains(test_uid)) {
                auto sample = m_testUIDMap.value(test_uid);
                if (status == "SamplingFinished") {
                    sample->setState(SampleStatus::SamplingFinished);
                }
            }
        } else if (obj.contains("order_uid")) {

        }
    }
}

