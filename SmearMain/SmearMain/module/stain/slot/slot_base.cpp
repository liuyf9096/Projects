#include "slot_base.h"
#include "sample/rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"
#include "f_common.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>

SlotBase::SlotBase(int pos, const QString &group, QObject *parent)
    : DModuleBase{QString("slot_%1").arg(pos), QString("slot_%1").arg(pos), parent}
    , mPos(pos)
    , mGroup(group)
    , m_request_id(0)
{
    m_isSolutionFilled = false;
    m_isCleaning = false;
    m_isDrained = true;
    m_isDetergentFilled = false;
    m_isClean = true;
    m_solutionDurationSec = 0;

    mMaxStainCount = 1;
    m_stainCount = 0;
    mExpiryTimeSec = 60000;

    m_water_soaktime_s = 0;
    m_alcohol_soaktime_s = 0;

    m_isAutoSolutionFill = false;
    mSlotInfo = QString("G:%1 Slot:%2").arg(mGroup).arg(mPos);

    m_durationTimer = new QTimer(this);
    m_durationTimer->setSingleShot(true);
    connect(m_durationTimer, &QTimer::timeout,
            this, &SlotBase::onDurationTimeout_slot);

    QJsonArray arr = mRecordDb->selectRecord("slot", QJsonObject({{"slot_pos", mPos}}));
    if (arr.count() > 0) {
        QJsonObject obj = arr.first().toObject();
        int isSolutionFilled = obj.value("isSolutionFilled").toInt();
        m_isSolutionFilled = isSolutionFilled > 0 ? true : false;
        if (m_isSolutionFilled == true) {
            QString start_time = obj.value("solution_startTime").toString();
            qDebug() << mSlotInfo << "solution:" << start_time;
            QDateTime t = QDateTime::fromString(start_time, "yyyy.MM.dd hh:mm:ss");
            if (start_time.isEmpty() == false && t.isValid()) {
                qint64 delta_sec = t.secsTo(QDateTime::currentDateTime());
                m_solutionDurationSec = delta_sec;
                qDebug() << mSlotInfo << "Solution Duration:" << m_solutionDurationSec << "sec.";
            }
        }
    }

    setDuration(45);
}

void SlotBase::setDuration(int sec)
{
    mDurationSec = sec;
    m_durationTimer->setInterval(sec * 1000);
}

bool SlotBase::isAvailable()
{
    if (m_slide == nullptr && m_isCleaning == false) {
        return true;
    }
    return false;
}

void SlotBase::putinSlide(const QString &sid)
{
    auto manager = RtSampleManager::GetInstance();
    if (m_slide == nullptr && manager->containSlide(sid)) {
        m_slide = manager->getSlide(sid);

        if (m_slide->isCancelled() && mGroup != "wash") {
            qDebug().noquote() << QString("handle cancel slot:[%1] slide:%2 pos:%3")
                                 .arg(mGroup, sid).arg(mPos);
            onDurationTimeout_slot();
        } else {
            m_durationTimer->start();
            qDebug().noquote() << QString("slot:[%1] slide:%2 pos:%3 stain start")
                                 .arg(mGroup, sid).arg(mPos);
        }

        handlePutinSlide();

        /* record */
        recordSlotSlideState(mPos, m_slide);
    }
}

void SlotBase::setRemainSlideSlot(const QString &slide_id)
{
    if (m_slide == nullptr && RtSampleManager::GetInstance()->containSlide(slide_id)) {
        m_slide = RtSampleManager::GetInstance()->getSlide(slide_id);
        qDebug().noquote() << QString("handle remain slide, group:[%1] slide:%2 pos:%3")
                             .arg(mGroup, m_slide->slide_id()).arg(mPos);

        onDurationTimeout_slot();
    }
}

void SlotBase::takeoutSlide(const QString &sid)
{
    if (m_slide->slide_id() == sid) {
        if (m_durationTimer->isActive()) {
            m_durationTimer->stop();
        }

        QJsonObject setObj;
        if (m_isSolutionFilled == true) {
            m_stainCount++;            
            setObj.insert("stainCount", m_stainCount);
        }
        setObj.insert("slide_id", "");
        setObj.insert("slide_uid", "");
        setObj.insert("sample_id", "");
        setObj.insert("sample_uid", "");
        mRecordDb->updateRecord("slot", setObj, {{"slot_pos", mPos}});

        handleTakeoutSlide();

        qInfo().noquote() << QString("slot:[%1] takeout slide:\"%2\" pos:%3").arg(mGroup, sid).arg(mPos);

        m_slide = nullptr;
    }
}

void SlotBase::onDurationTimeout_slot()
{
    qDebug() << __FUNCTION__ << mPos << "stain timeout";

    m_durationTimer->stop();

    if (m_slide) {
        m_slide->removeStainProcessOne(mGroup);

        QJsonObject obj;
        obj.insert("from_groupid", mGroup);
        obj.insert("from_pos", mPos);
        obj.insert("sid", m_slide->slide_id());

        QString to_groupid = m_slide->getNextStainProcess();
        if (!to_groupid.isEmpty()) {
            obj.insert("to_groupid", to_groupid);
        } else {
            obj.insert("to_groupid", "recycleBox");
        }
        emit onGripperRequest_signal(obj);

        qDebug() << "Gripper Request:" << obj;
    }

    onDurationTimeout();
}
