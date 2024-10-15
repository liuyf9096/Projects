#include "rt_sample.h"
#include "rt_sample_manager.h"
#include "exception/exception_center.h"

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
    {SampleStatus::Review_Done, "Review_Done"},
    {SampleStatus::Need_Smear, "Need_Smear"},
    {SampleStatus::Send_To_Smear, "Send_To_Smear"},
    {SampleStatus::Smear_Done, "Smear_Done"}
};

RtSample::RtSample(const QString &sid, QObject *parent)
    : QObject(parent)
    , mSid(sid)
    , m_isCanceled(false)
{
    qDebug() << "New Sample:" << sid;
    m_sample_id = sid; //for test todo

    m_isSmearMode = false;
    m_isEmergency = false;
    m_reviewMode = ReviewMode::Unknown;

    RtSampleManager::GetInstance()->sql_record(sid);
}

RtSample::~RtSample()
{
    qDebug() << __FUNCTION__ << mSid;
}

void RtSample::setRackInfo(const QString &rackid, int pos)
{
    mRackId = rackid;
    mRackPos = pos;

    QJsonObject obj;
    obj.insert("rack_id", rackid);
    obj.insert("rack_pos", pos);

    RtSampleManager::GetInstance()->sql_record(mSid, obj);
}

void RtSample::setBarcode(const QString &barcode)
{
    mBarcode = barcode;

    RtSampleManager::GetInstance()->sql_record(mSid, {{"barcode", mBarcode}});
}

void RtSample::setSampleID(const QString &sample_id)
{
    m_sample_id = sample_id;

    RtSampleManager::GetInstance()->sql_record(mSid, {{"sample_id", m_sample_id}});
}

void RtSample::setProgramList(const QStringList &list)
{
    m_programList = list;

    RtSampleManager::GetInstance()->sql_record(mSid, {{"program", list.join(", ")}});
}

void RtSample::clearProgramList()
{
    m_programList.clear();
    RtSampleManager::GetInstance()->sql_record(mSid, {{"program", ""}});
}

QString RtSample::nextProgram()
{
    if (!m_programList.isEmpty()) {
        return m_programList.first();
    }
    return QString();
}

void RtSample::setDoingProgram(const QString &program)
{
    m_doingProgram = program;
    qDebug().noquote() << QString("Sample['%1'] Doing Program:'%2'").arg(mSid).arg(program);
}

void RtSample::removeProgram(const QString &program)
{
    m_programList.removeOne(program);
    qDebug().noquote() << QString("Sample['%1'] Update Program list:").arg(mSid) << m_programList;
}

void RtSample::removeDoingProgram()
{
    removeProgram(m_doingProgram);
}

void RtSample::setOrder_uid(const QString &uid)
{
    m_order_uid = uid;
    m_orderInfo.insert("order_uid", m_order_uid);

    RtSampleManager::GetInstance()->sql_record(mSid, {{"order_uid", m_order_uid}});
}

void RtSample::setTest_uid(const QString &uid)
{
    m_test_uid = uid;
    m_orderInfo.insert("test_uid", uid);
    m_test_uid_list.append(uid);

    RtSampleManager::GetInstance()->sql_record(mSid, {{"test_uid", m_test_uid_list.join(",")}});
}

void RtSample::setRetestMode(bool isDone, const QString &mode)
{
    if (isDone == false) {
        m_reviewMode = ReviewMode::Done;
    } else {
        if (mode == "auto") {
            m_reviewMode = ReviewMode::Auto_Retest;
        } else if (mode == "local") {
            m_reviewMode = ReviewMode::Local_Retest;
        } else if (mode == "other") {
            m_reviewMode = ReviewMode::Other_Retest;
        } else if (mode == "micro") {
            m_reviewMode = ReviewMode::Smear;
        } else if (mode == "smear") {
            m_reviewMode = ReviewMode::Smear;
        } else if (mode == "none") {
            m_reviewMode = ReviewMode::Done;
        } else {
            qDebug() << "can NOT handle" << __FUNCTION__ << mode;
        }
    }

    setState(SampleStatus::Review_Sample, 1);

    RtSampleManager::GetInstance()->sql_record(mSid, {{"retest_mode", mode}});
    qDebug() << mSid << "set Retest isDone:" << isDone << "mode:" << mode;
}

void RtSample::doReviewMode(bool isClassify, bool isSmear)
{
    switch (m_reviewMode) {
    case ReviewMode::Auto_Retest:
    case ReviewMode::Local_Retest:
    case ReviewMode::Other_Retest:
        if (isClassify) {
            m_test_uid.clear();
            m_programList.append("5-classify");
            setState(SampleStatus::Send_To_Test, 0);
            setState(SampleStatus::SamplingFinished, 0);
            setState(SampleStatus::Recycle_To_Rack, 0);
            setState(SampleStatus::Test_Finished, 0);
        } else {
            setState(SampleStatus::Review_Done, 1);
            qDebug() << "can NOT handle Sample Retest:" << mSid << "5-classify";
            RtSampleManager::GetInstance()->sendOperationInfo("Unavailable_Analyse_Station", mSid);
        }
        break;
    case ReviewMode::Smear:
        if (isSmear) {
            m_test_uid.clear();
            m_programList.append("smear");
            setState(SampleStatus::Send_To_Test, 0);
            setState(SampleStatus::SamplingFinished, 0);
            setState(SampleStatus::Recycle_To_Rack, 0);
            setState(SampleStatus::Test_Finished, 0);

            setState(SampleStatus::Need_Smear, 1);
            setState(SampleStatus::Send_To_Smear, 0);
        } else {
            setState(SampleStatus::Review_Done, 1);
            qDebug() << "can NOT handle Sample Smear:" << mSid << "smear";
            RtSampleManager::GetInstance()->sendOperationInfo("Unavailable_Smear_Station", mSid);
        }
        break;
    case ReviewMode::Done:
        setState(SampleStatus::Review_Done, 1);
        break;
    default:
        setState(SampleStatus::Review_Done, 1);
        break;
    }

    setState(SampleStatus::Review_Sample, 1);

    qDebug() << mSid << "set Do Review Mode. Program:" << m_programList;

    RtSampleManager::GetInstance()->sql_record(mSid, {{"program", m_programList.join(", ")}});
}

void RtSample::setSmearMode()
{
    m_isSmearMode = true;
    setState(SampleStatus::Send_To_Test, 1);
    setState(SampleStatus::SamplingFinished, 1);
    setState(SampleStatus::Recycle_To_Rack, 1);
    setState(SampleStatus::Test_Finished, 1);
}

bool RtSample::setOrderInfo(const QJsonObject &obj)
{
    if (obj.contains("order_uid")) {
        m_orderInfo = obj;

        QString order_uid = obj.value("order_uid").toString();
        setOrder_uid(order_uid);

        mBloodType = obj.value("blood_sample_pattern").toString();
        m_testMode = obj.value("analysis_pattern").toString();
        QString sample_id = obj.value("sample_id").toString();
        setSampleID(sample_id);

        /* self info */
        m_orderInfo.insert("order_source", "DMU");
        m_orderInfo.insert("rack_id", rack_id());
        m_orderInfo.insert("rack_pos", rack_pos());
        m_orderInfo.insert("barcode", barcode());
        if (m_sample_id.isEmpty()) {
            m_sample_id = m_order_uid;
            m_orderInfo.insert("sample_id", m_sample_id);
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
    m_orderInfo.insert("order_uid", mSid);
    m_orderInfo.insert("test_uid", mSid);
    m_orderInfo.insert("sample_id", m_sample_id);
    m_orderInfo.insert("sample_entry_pattern", "Auto");
    m_orderInfo.insert("rack_id", mRackId);
    m_orderInfo.insert("rack_pos", mRackPos);

    m_order_uid = mSid;
    m_test_uid = mSid;
}

void RtSample::addOrderUIDInfo(const QString &key, const QJsonValue &value)
{
    m_orderInfo.insert(key, value);
}

void RtSample::setState(SampleStatus status, int value)
{
    m_statusMap.insert(StatusMap.value(status), value);

    RtSampleManager::GetInstance()->sql_recordStatus(mSid, status, value);
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
