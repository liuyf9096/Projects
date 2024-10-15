#include "rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"
#include "f_common.h"
#include <QDateTime>
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
    bool clear_db = FCommon::GetInstance()->getConfigValue("general", "clear_db_onstart").toBool();
    if (clear_db) {
        m_sid = 1;
        mLogDb->deleteRecord("samples");
    } else {
        int count = mLogDb->count("samples");
        qDebug() << "Sample Record Count:" << count;
        m_sid = count + 1;
    }

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onClientsMessageNotification_signal,
            this, &RtSampleManager::handleSampleState_slot);
    connect(center, &FMessageCenter::onClientsMessageRequestPacket_signal,
            this, &RtSampleManager::handleSampleState_slot);
    connect(center, &FMessageCenter::onDMUMessageResultPacket_signal,
            this, &RtSampleManager::handleSampleOrder_slot);
}

QSharedPointer<RtSample> RtSampleManager::NewSample()
{
    QString sid = QString("S%1").arg(m_sid++);
    auto sample = QSharedPointer<RtSample>(new RtSample(sid));
    sample->setBloodType("WholeBlood");
    m_sampleMap.insert(sid, sample);

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
        removeSampleOne(sample);
    }
}

void RtSampleManager::removeSampleOne(QSharedPointer<RtSample> sample)
{
    QString order_uid = sample->order_uid();
    if (m_orderUIDMap.contains(order_uid)) {
        m_orderUIDMap.remove(order_uid);
    }

    foreach (auto test_uid, sample->test_uid_list()) {
        if (m_testUIDMap.contains(test_uid)) {
            m_testUIDMap.remove(test_uid);
        }
    }
    m_sampleMap.remove(sample->sid());

    qInfo() << __FUNCTION__ << sample->sid();
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

bool RtSampleManager::requestSampleOrderUid(QSharedPointer<RtSample> sample)
{
    bool isconnect = FMessageCenter::GetInstance()->isDMUConnected();
    if (isconnect == false) {
        sample->setDefaultOrderInfo();
        m_orderUIDMap.insert(sample->order_uid(), sample);
        m_testUIDMap.insert(sample->test_uid(), sample);
        qDebug() << "Default Sample order_uid:" << sample->sid() << "test_uid:"<< sample->test_uid();
        return false;
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

        quint64 id = FMessageCenter::GetInstance()->sendDMUMessage(p);
        m_queryOrderUIDMap.insert(id, sample->sid());
        return true;
    }
}

bool RtSampleManager::requestSampleTestUid(const QString &dev_id, QSharedPointer<RtSample> sample)
{
    bool isconnect = FMessageCenter::GetInstance()->isDMUConnected();
    if (isconnect == true) {
        JPacket p(PacketType::Request);
        p.device = "LabXpert";
        p.module = "Order";
        p.api = "AssignTestUID";

        QJsonObject obj;
        obj.insert("order_uid", sample->order_uid());
        obj.insert("device_id", dev_id);
        obj.insert("sample_id", sample->SampleID());
        obj.insert("barcode", sample->barcode());
        p.paramsValue = obj;

        quint64 id = FMessageCenter::GetInstance()->sendDMUMessage(p);
        m_queryTestUIDMap.insert(id, sample->sid());
        return true;
    }
    return false;
}

void RtSampleManager::setSampleReviewTask(const JPacket &request)
{
    JPacket p;
    p.id = request.id;

    QJsonObject obj = request.paramsValue.toObject();
    QString order_uid = obj.value("order_uid").toString();
    bool isRetest = obj.value("retest").toBool();

    QJsonObject retestInfoObj = obj.value("retest_info").toObject();
    QString mode = retestInfoObj.value("mode").toString();
    QJsonArray requirements = retestInfoObj.value("requirements").toArray();

    if (m_orderUIDMap.contains(order_uid)) {
        auto sample = m_orderUIDMap.value(order_uid);
        Q_ASSERT(sample);

        sample->setRetestMode(isRetest, mode);
        sample->setRetestRequirement(requirements);

        /* Start Review Task */
        sample->doReviewMode(true, true);

        p.type = PacketType::Result;
        p.resValue = true;
    } else {
        p.type = PacketType::Error;
        p.errorCode = 90;
        p.errorMessage = "can NOT find sample with uid:" + order_uid;
    }
    FMessageCenter::GetInstance()->sendDMUMessage(p);
}

void RtSampleManager::sendOperationInfo(const QString &error_id, const QString &sid)
{
    QJsonObject msgObj;
    msgObj.insert("datetime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    QJsonObject whereObj;
    whereObj.insert("device", "main");
    whereObj.insert("exception_id", error_id);

    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    QJsonArray sel = db->selectRecord("exception", whereObj);
    if (sel.count() > 0) {
        QJsonObject obj = sel.first().toObject();
        QString message = obj.value("message").toString();
        int level =  obj.value("level").toInt();
        int code =  obj.value("code").toInt();

        msgObj.insert("message", message);
        msgObj.insert("level", level);
        msgObj.insert("code", code);
    }

    if (m_sampleMap.contains(sid)) {
        auto sample = m_sampleMap.value(sid);
        msgObj.insert("sample_id", sample->SampleID());
        msgObj.insert("order_uid", sample->order_uid());
        msgObj.insert("sample_pos", QString("%1-%2").arg(sample->rack_id()).arg(sample->rack_pos() + 1));
    }

    JPacket p(PacketType::Notification);
    p.module = QStringLiteral("Operation");
    p.api = QStringLiteral("OperationInfo");
    p.paramsValue = msgObj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RtSampleManager::clearQueryMessage()
{
    m_queryOrderUIDMap.clear();
    m_queryTestUIDMap.clear();
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

                    if (sample->isSmearMode() == false) {
                        sample->setProgramList({"5-classify"});
//                        sample->setProgramList({"5-classify", "smear"});
                    }
                    sendSamplePath(sample, "import");
                }
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

void RtSampleManager::handleSampleState_slot(const QString &client_id, const JPacket &note)
{
    if (note.api == "Sample") {
        auto obj = note.paramsValue.toObject();
        if (obj.contains("test_uid")) {
            QString test_uid = obj.value("test_uid").toString();
            QString status = obj.value("status").toString();
            if (m_testUIDMap.contains(test_uid)) {
                auto sample = m_testUIDMap.value(test_uid);
                if (status == "SamplingFinished") {
                    sample->setState(SampleStatus::SamplingFinished, 1);

                    if (note.type == PacketType::Request) {
                        JPacket p(PacketType::Result, note.id);
                        FMessageCenter::GetInstance()->sendClientMessage(client_id, p);
                    }
                }
            }
        } else if (obj.contains("order_uid")) {

        }
    }
}

void RtSampleManager::sendSamplePath(QSharedPointer<RtSample> sample, const QString &node_id)
{
    JPacket p(PacketType::Notification);
    p.device = "United";
    p.module = "SamplePath";
    p.api = "SetSamplePath";

    QJsonObject obj;
    obj.insert("order_uid", sample->order_uid());
    obj.insert("test_uid", sample->test_uid());
    obj.insert("sample_id", sample->SampleID());
    obj.insert("node_id", node_id);
    if (node_id == "import") {
        obj.insert("rack_pos", sample->rack_pos());
        obj.insert("rack_id", sample->rack_id());
    }
    obj.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
    FMessageCenter::GetInstance()->sendDMUMessage(p);
}

