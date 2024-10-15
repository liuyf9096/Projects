#include "rt_sample.h"
#include "rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"

#include <QJsonObject>
#include <QDebug>

QMap<SampleStatus, QString> RtSample::SampleStatusMap = {
    {SampleStatus::Scaned, "Scaned"},
    {SampleStatus::Sending_To_Test, "Sending_To_Test"},
    {SampleStatus::Send_To_Test, "Send_To_Test"},
    {SampleStatus::Mixing, "Mixing"},
    {SampleStatus::Sampling, "Sampling"},
    {SampleStatus::Test_Finished, "Test_Finished"},
    {SampleStatus::Prepare_Blood, "Prepare_Blood"},
    {SampleStatus::Ret_Hatch, "Ret_Hatch"},
    {SampleStatus::Recycle_To_Rack, "Recycle_To_Rack"},
    {SampleStatus::Add_Sample, "Add_Sample"},
    {SampleStatus::Abort, "Abort"}
};

RtSample::RtSample(const QString &sid, QObject *parent)
    : QObject(parent)
    , mSid(sid)
{
    qDebug() << "New Sample:" << sid;

    _init();

    sql_record();
}

RtSample::~RtSample()
{
    qDebug() << __FUNCTION__ << mSid;
}

void RtSample::_init()
{
    m_isEmergency = false;
    m_isMiniBlood = false;
    m_isRet = false;
    m_isCapped = true;
    m_isNeedMix = true;
    m_isCanceled = false;

    m_isPrintEnable = true;
    m_isSmearEnable = true;
    m_isStainEnable = true;

    mRackPos = -1;

    m_smearCount = 1;
    m_smearLevel = 1;
    m_hct = 26.0;
    m_viscosity = 0;
    m_add_volume = 200;

    // 3600,1200,800,2000,9000,2100,2000,2000,6000,2000
    m_smearParams << 3600 << 1200 << 800 << 2000 << 9000 << 2100 << 2000 << 2000 << 6000 << 2000;
}

void RtSample::setRackInfo(const QString &rackid, int pos)
{
    mRackId = rackid;
    mRackPos = pos;

    QJsonObject obj;
    obj.insert("rack_id", rackid);
    obj.insert("rack_pos", pos);
    sql_record(obj);
}

void RtSample::setBarcode(const QString &barcode)
{
    mBarcode = barcode;

    QJsonObject obj;
    obj.insert("barcode", barcode);
    sql_record(obj);
}

void RtSample::setSampleUID(const QString &uid)
{
    mSampleUID = uid;

    QJsonObject obj;
    obj.insert("sample_uid", uid);
    sql_record(obj);
}

void RtSample::setSampleID(const QString &id)
{
    m_sample_id = id;

    QJsonObject obj;
    obj.insert("sample_id", id);
    sql_record(obj);
}

void RtSample::setTest_uid(const QString &uid)
{
    m_test_uid = uid;

    QJsonObject obj;
    obj.insert("test_uid", uid);
    sql_record(obj);
}

void RtSample::setOrder_uid(const QString &uid)
{
    m_order_uid = uid;

    QJsonObject obj;
    obj.insert("order_uid", uid);
    sql_record(obj);
}

void RtSample::setOrderInfo(const QJsonObject &obj)
{
    m_orderInfo = obj;

    m_test_uid = obj.value("test_uid").toString();
    m_order_uid = obj.value("order_uid").toString();
    mBloodType = obj.value("blood_sample_pattern").toString();
    mRackId = obj.value("rack_id").toString();
    mRackPos = obj.value("rack_pos").toInt();
    mBarcode = obj.value("barcode").toString();

    setOrder_uid(m_order_uid);
    setTest_uid(m_test_uid);

    m_sample_id = obj.value("sample_id").toString();
}

void RtSample::setSmearCount(int count)
{
    if (m_isRet == false && count > 0 && count <= 10) {
        m_smearCount = count;
        qInfo().noquote() << QString("%1 Semar Count:%2").arg(mSid).arg(count);
    }
}

void RtSample::setHtc(double htc)
{
    m_hct = htc;
    qInfo().noquote() << QString("%1 HCT:%2").arg(mSid).arg(htc);
}

void RtSample::setMiniBlood(bool mini)
{
    m_isMiniBlood = mini;
    if (mini) {
        qDebug() << mSid << "mini blood";
    } else {
        qDebug() << mSid << "whold blood";
    }
}

void RtSample::setEmergency(bool emergency)
{
    m_isEmergency = emergency;

    QJsonObject obj;
    obj.insert("is_emergency", emergency);
    sql_record(obj);
}

void RtSample::setIsNeedMix(bool isNeedMix)
{
    m_isNeedMix = isNeedMix;
}

void RtSample::setRet(bool ret)
{
    m_isRet = ret;
    if (ret == true) {
        qDebug() << mSid << "is Ret.";
    }

    QJsonObject obj;
    obj.insert("is_ret", ret ? 1 : 0);
    sql_record(obj);
}

void RtSample::setCapped(bool capped)
{
    m_isCapped = capped;
    qDebug() << mSid << "Capped:" << capped;
}

bool RtSample::getSmearParams()
{
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("smearparams");
    if (db) {
        QString query = QString("min <= %1 AND max >= %1").arg(m_hct);
        QString table = m_isCapped ? "capped" : "uncapped";

        table = "uncapped"; // test

        QJsonArray arr = db->selectRecord(table, query);
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            qDebug() << QString("sample:%1 hct:%2 smearparams:").arg(mSid).arg(m_hct) << obj;

            z_hight = obj.value("z_hight").toInt(940);
            start_pos = obj.value("start_pos").toInt(1392);
            expend_time = obj.value("expend_time").toInt(1500);
            start_speed = obj.value("start_speed").toInt(2400);
            max_speed = obj.value("max_speed").toInt(4300);
            length = obj.value("length").toInt(2200);
            z_wait = obj.value("z_wait").toInt(700);
            z_start_speed = obj.value("z_start_speed").toInt(2000);
            z_max_speed = obj.value("z_max_speed").toInt(6000);
            z_range = obj.value("z_range").toInt(2000);
            m_smearParams = {z_hight, start_pos, expend_time, start_speed, max_speed,
                             length, z_wait, z_start_speed, z_max_speed, z_range};

            m_add_volume = obj.value("add_volume").toInt(170);
            qDebug() << mSid << "SmearParams:" << m_smearParams;
            return true;
        } else {
            qWarning() << "can NOT match the corresponding smear parameters.";
        }
    }
    return false;
}

void RtSample::setPrintEnable(bool en)
{
    m_isPrintEnable = en;

    QJsonObject obj;
    obj.insert("is_print", en);
    sql_record(obj);
}

void RtSample::setSmearEnable(bool en)
{
    m_isSmearEnable = en;

    QJsonObject obj;
    obj.insert("is_smear", en);
    sql_record(obj);
}

void RtSample::setStainEnable(bool en)
{
    m_isStainEnable = en;

    QJsonObject obj;
    obj.insert("is_stain", en);
    sql_record(obj);
}

void RtSample::setPrintInfo(const QJsonObject &obj)
{
    mPrintInfoObj = obj;
    qDebug() << "set sample print into:" << obj;

    QString str;
    str.append("1:");
    str.append(obj.value("line1").toString());
    str.append("; 2:");
    str.append(obj.value("line2").toString());
    str.append("; 3:");
    str.append(obj.value("line3").toString());
    str.append(";");

    QJsonObject setObj;
    setObj.insert("print_info", str);
    sql_record(setObj);
}

void RtSample::setDefaultPrintInfo()
{
    if (mSampleUID.isEmpty() == false) {
        mPrintInfoObj.insert("line1", mSampleUID);
    } else {
        mPrintInfoObj.insert("line1", m_test_uid);
    }

    mPrintInfoObj.insert("line2", "date");
    mPrintInfoObj.insert("line3", "time");
}

void RtSample::setStainProcessList(const QStringList &list)
{
    m_stainProcessList = list;

    QJsonObject obj;
    obj.insert("process", list.join(", "));
    sql_record(obj);
}

void RtSample::setViscosity(int viscosity)
{
    m_viscosity = viscosity;
    qDebug() << "set viscosity" << viscosity;

    QJsonObject obj;
    obj.insert("viscosity", viscosity);
    sql_record(obj);
}

void RtSample::setSmearParams(const QJsonArray &arr)
{
    m_smearParams = arr;
    qDebug() << "set SmearParams:" << m_smearParams;
}

void RtSample::setStatus(SampleStatus status, int value)
{
    m_statusMap.insert(SampleStatusMap.value(status), value);

    sql_recordSampleStatus(status, value);

    if (status == SampleStatus::Abort) {
        if (getStatus(SampleStatus::Sending_To_Test) == 0) {
            RtSampleManager::GetInstance()->sendUISampleStatus(mSid, SampleProcessState::Canceled);
        }
    }
}

int RtSample::getStatus(SampleStatus status)
{
    if (m_statusMap.contains(SampleStatusMap.value(status))) {
        return m_statusMap.value(SampleStatusMap.value(status));
    }
    return 0;
}

void RtSample::sql_record(const QJsonObject &setObj)
{
    FSqlDatabase *log = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (setObj.isEmpty()) {
        log->insertRecord("samples", {{"sid", mSid}});
    } else {
        log->updateRecord("samples", setObj, {{"sid", mSid}});
    }
}

void RtSample::sql_recordSampleStatus(SampleStatus status, int value)
{
    if (SampleStatusMap.contains(status)) {
        QJsonObject obj;
        obj.insert(SampleStatusMap.value(status), value);
        sql_record(obj);
    }
}
