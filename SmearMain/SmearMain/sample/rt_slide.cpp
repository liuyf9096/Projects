#include "rt_slide.h"
#include "sql/f_sql_database_manager.h"
#include <QDebug>

QMap<SmearStatus, QString> RtSlide::SmearStatusMap = {
    {SmearStatus::Get_New_Slide, "Get_New_Slide"},
    {SmearStatus::Print_Info_Finish, "Print_Info_Finish"},
    {SmearStatus::WaitF_Add_Blood, "WaitF_Add_Blood"},
    {SmearStatus::Add_Sample, "Add_Sample"},
    {SmearStatus::Smear, "Smear"},
    {SmearStatus::Wash_Blade, "Wash_Blade"},
    {SmearStatus::Send_To_StainCart, "Send_To_StainCart"}
};

QMap<StainStatus, QString> RtSlide::StainStatusMap = {
    {StainStatus::Fix, "Fix"},
    {StainStatus::A1, "A1"},
    {StainStatus::C1, "C1"},
    {StainStatus::C2, "C2"},
    {StainStatus::Wash, "Wash"},
    {StainStatus::Recycle, "Recycle"}
};

RtSlide::RtSlide(const QString &slide_id, QObject *parent)
    : QObject{parent}
    , mSlideId(slide_id)
    , mStainPos(0)
    , m_isSendRequest(false)
{
    qDebug() << "New Slide:" << slide_id;

    m_isPrintEnable = true;
    m_isSmearEnable = true;
    m_isStainEnable = true;
    m_isRemained = false;
    m_isCancelled = false;

    sql_record();

//    setStainProcessList({"transfer"});
}

RtSlide::~RtSlide()
{
    m_sample = nullptr;
    qDebug() << __FUNCTION__ << mSlideId;
}

void RtSlide::setSlideUID(const QString &uid)
{
    mSlideUID = uid;

    QJsonObject obj;
    obj.insert("slide_uid", uid);
    sql_record(obj);
}

void RtSlide::setSampleUID(const QString &uid)
{
    mSampleUID = uid;

    QJsonObject obj;
    obj.insert("sample_uid", uid);
    sql_record(obj);
}

void RtSlide::setSampleID(const QString &id)
{
    m_sample_id = id;

    QJsonObject obj;
    obj.insert("sample_id", id);
    sql_record(obj);
}

void RtSlide::setQRcode(const QString &code)
{
    m_qrcode = code;

    QJsonObject obj;
    obj.insert("qr_code", code);
    sql_record(obj);

    qDebug() << mSlideId << "set slide QRcode:" << m_qrcode;
}

void RtSlide::setQRcode(const QString &code1, const QString &code2, bool isRet)
{
    if (code1.isEmpty() == false) {
        m_qrcode = code1;
        if (code2.isEmpty() == false) {
            m_qrcode.append(":");
            if (code2.startsWith("order")) {
                QString c2 = code2.right(code2.length() - 6);
                m_qrcode.append(c2);
            }

            if (isRet == true) {
                m_qrcode.append(":R");
            }
        }
    }
    if (m_qrcode.length() > 20) {
        m_qrcode = m_qrcode.right(20);
    }

    QJsonObject obj;
    obj.insert("qr_code", m_qrcode);
    sql_record(obj);

    qDebug() << mSlideId << "set slide combQRcode:" << m_qrcode;
}

void RtSlide::setRemained()
{
    m_isRemained = true;
    m_isCancelled = true;

    QJsonObject obj;
    obj.insert("is_remained", 1);
    sql_record(obj);

    qDebug() << mSlideId << "setRemained.";
}

void RtSlide::setStainProcessList(const QStringList &list)
{
    m_stainProcessList = list;

    QJsonObject obj;
    obj.insert("process", m_stainProcessList.join(", "));
    sql_record(obj);
}

void RtSlide::addStainProcess(const QString &process)
{
    m_stainProcessList.append(process);

    QJsonObject obj;
    obj.insert("process", m_stainProcessList.join(", "));
    sql_record(obj);
}

void RtSlide::removeStainProcessOne(const QString &step)
{
    m_stainProcessList.removeOne(step);
}

QString RtSlide::getNextStainProcess()
{
    if (m_stainProcessList.isEmpty()) {
        return QString();
    } else {
        return m_stainProcessList.first();
    }
}

void RtSlide::setRet(bool ret)
{
    m_isRet = ret;

    QJsonObject obj;
    obj.insert("is_ret", ret);
    sql_record(obj);
}

void RtSlide::setPrintEnable(bool en)
{
    m_isPrintEnable = en;

    QJsonObject obj;
    obj.insert("is_print", en);
    sql_record(obj);
}

void RtSlide::setSmearEnable(bool en)
{
    m_isSmearEnable = en;

    QJsonObject obj;
    obj.insert("is_smear", en);
    sql_record(obj);
}

void RtSlide::setStainEnable(bool en)
{
    m_isStainEnable = en;

    QJsonObject obj;
    obj.insert("is_stain", en);
    sql_record(obj);
}

void RtSlide::setPrintInfo(const QJsonObject &obj)
{
    mPrintInfoObj = obj;
    qDebug() << mSlideId << "set print info:" << obj;

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

void RtSlide::setDefaultPrintInfo()
{
    mPrintInfoObj.insert("line1", mSlideId);
    mPrintInfoObj.insert("line2", "date");
    mPrintInfoObj.insert("line3", "time");
}

void RtSlide::setStatus(SmearStatus status, int value)
{
    m_statusMap.insert(SmearStatusMap.value(status), value);

    sql_recordSmearStatus(status, value);
}

int RtSlide::getStatus(SmearStatus status)
{
    if (m_statusMap.contains(SmearStatusMap.value(status))) {
        return m_statusMap.value(SmearStatusMap.value(status));
    }
    return 0;
}

void RtSlide::setStatus(StainStatus status, int value)
{
    m_statusMap.insert(StainStatusMap.value(status), value);

    sql_recordStainStatus(status, value);
}

int RtSlide::getStatus(StainStatus status)
{
    if (m_statusMap.contains(StainStatusMap.value(status))) {
        return m_statusMap.value(StainStatusMap.value(status));
    }
    return 0;
}

void RtSlide::sql_record(const QJsonObject &setObj)
{
    FSqlDatabase *log = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (setObj.isEmpty()) {
        log->insertRecord("slides", {{"slide_id", mSlideId}});
    } else {
        log->updateRecord("slides", setObj, {{"slide_id", mSlideId}});
    }
}

void RtSlide::sql_recordSmearStatus(SmearStatus status, int value)
{
    if (SmearStatusMap.contains(status)) {
        QJsonObject obj;
        obj.insert(SmearStatusMap.value(status), value);
        sql_record(obj);
    }
}

void RtSlide::sql_recordStainStatus(StainStatus status, int value)
{
    if (StainStatusMap.contains(status)) {
        QJsonObject obj;
        obj.insert(StainStatusMap.value(status), value);
        sql_record(obj);
    }
}

void RtSlide::sql_recordStainStatus(const QString &key, const QString &value)
{
    QJsonObject obj;
    obj.insert(key, value);
    sql_record(obj);
}

