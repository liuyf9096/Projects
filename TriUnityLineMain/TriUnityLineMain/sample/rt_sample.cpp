#include "rt_sample.h"
#include "rt_sample_manager.h"

#include <QJsonObject>
#include <QDebug>

QMap<SampleStatus, QString> RtSample::StatusMap = {
    {SampleStatus::Emergency, "Emergency"},
    {SampleStatus::Scaned, "Scaned"},
    {SampleStatus::Send_To_Test, "Send_To_Test"},
    {SampleStatus::SamplingFinished, "Sampling"},
    {SampleStatus::Test_Finished, "Test_Finished"},
    {SampleStatus::Recycle_To_Rack, "Recycle_To_Rack"},
    {SampleStatus::Review_Sample, "Review_Sample"},
    {SampleStatus::Need_Smear, "Need_Smear"},
    {SampleStatus::Send_To_Smear, "Send_To_Smear"},
    {SampleStatus::Smear_Done, "Smear_Done"}
};

RtSample::RtSample(const QString &sid, QObject *parent)
    : QObject(parent)
    , mSampleId(sid)
    , m_isCanceled(false)
{
    qDebug() << "New Sample:" << sid;
    mSampleSN = sid; //for test todo

    m_isEmergency = false;
    RtSampleManager::GetInstance()->sql_record(sid);
}

RtSample::~RtSample()
{
    qDebug() << __FUNCTION__ << mSampleId;
}

void RtSample::setRackInfo(const QString &rackid, int pos)
{
    mRackId = rackid;
    mRackPos = pos;

    QJsonObject obj;
    obj.insert("rack_id", rackid);
    obj.insert("rack_pos", pos);

    RtSampleManager::GetInstance()->sql_record(mSampleId, obj);
}

void RtSample::setBarcode(const QString &barcode)
{
    mBarcode = barcode;

    QJsonObject obj;
    obj.insert("barcode", barcode);
    RtSampleManager::GetInstance()->sql_record(mSampleId, obj);
}

void RtSample::setProgramList(const QStringList &list)
{
    m_programList = list;

    QJsonObject obj;
    obj.insert("program", list.join(", "));
    RtSampleManager::GetInstance()->sql_record(mSampleId, obj);
}

QString RtSample::program()
{
    if (!m_programList.isEmpty()) {
        return m_programList.first();
    }
    return QString();
}

void RtSample::setOrder_uid(const QString &uid)
{
    m_order_uid = uid;
    m_orderInfo.insert("order_uid", m_order_uid);
}

void RtSample::setTest_uid(const QString &uid)
{
    m_test_uid = uid;
    m_orderInfo.insert("test_uid", uid);
}

bool RtSample::setOrderInfo(const QJsonObject &obj)
{
    if (obj.contains("order_uid")) {
        m_orderInfo = obj;

        m_order_uid = obj.value("order_uid").toString();
        mBloodType = obj.value("blood_sample_pattern").toString();
        m_testMode = obj.value("analysis_pattern").toString();
        mSampleSN = obj.value("sample_id").toString();

        /* self info */
        m_orderInfo.insert("order_source", "DMU");
        m_orderInfo.insert("rack_id", rack_id());
        m_orderInfo.insert("rack_pos", rack_pos());
        m_orderInfo.insert("barcode", barcode());
        if (mSampleSN.isEmpty()) {
            mSampleSN = m_order_uid;
            m_orderInfo.insert("sample_id", mSampleSN);
        }

        return true;
    } else {
        qDebug() << "OrderUID Missing." << obj << sid();
    }
    return false;
}

void RtSample::setDefaultOrderInfo()
{
    m_orderInfo.insert("analysis_pattern", "CD");
    m_orderInfo.insert("order_source", "united");
    m_orderInfo.insert("blood_sample_pattern", "WholeBlood");
    m_orderInfo.insert("order_type", "analysis");
    m_orderInfo.insert("order_uid", mSampleId);
    m_orderInfo.insert("test_uid", mSampleId);
    m_orderInfo.insert("sample_id", mSampleSN);
    m_orderInfo.insert("sample_entry_pattern", "Auto");
    m_orderInfo.insert("rack_id", mRackId);
    m_orderInfo.insert("rack_pos", mRackPos);

    m_order_uid = mSampleId;
    m_test_uid = mSampleId;
}

void RtSample::addOrderUIDInfo(const QString &key, const QJsonValue &value)
{
    m_orderInfo.insert(key, value);
}

void RtSample::setState(SampleStatus status, int value)
{
    m_statusMap.insert(StatusMap.value(status), value);

    RtSampleManager::GetInstance()->sql_recordStatus(mSampleId, status, value);
}

void RtSample::setState(const QString &status, int value)
{
    m_statusMap.insert(status, value);
}

int RtSample::getState(SampleStatus status)
{
    if (m_statusMap.contains(StatusMap.value(status))) {
        return m_statusMap.value(StatusMap.value(status));
    }
    return 0;
}

int RtSample::getState(const QString &status)
{
    if (m_statusMap.contains(status)) {
        return m_statusMap.value(status);
    }
    return 0;
}

bool RtSample::isProcessDone(SampleStatus status)
{
    if (getState(status) > 0) {
        return true;
    }
    return false;
}



