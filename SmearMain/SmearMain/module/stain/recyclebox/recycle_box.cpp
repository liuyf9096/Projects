#include "recycle_box.h"
#include "sample/rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"
#include <QDebug>

RecycleBox::RecycleBox(const QString &box_id, int from, int to, QObject *parent)
    : QObject{parent}
    , m_boxid(box_id)
{
    m_isClearMode = false;
    m_isEjected = false;
    m_isUnloadedBox = false;

    if (from < to) {
        for (int i = from; i <= to; ++i) {
            mPosList.append(i);
        }
    } else if (from > to) {
        for (int i = from; i >= to; --i) {
            mPosList.append(i);
        }
    } else {
        qWarning() << "Always Full! from=to:" << from;
    }

    m_maxCount = mPosList.count();
    qDebug().noquote() << QString("[%1] :").arg(box_id) << mPosList;

    JPacket p(PacketType::Notification);
    p.module = "RecycleBox";
    p.api = "NewBox";

    QJsonObject obj;
    obj.insert("box_id", m_boxid);
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

RecycleBox::~RecycleBox()
{
    auto mgr = RtSampleManager::GetInstance();
    foreach (auto slide, mSlideMap) {
        mgr->removeSlideOne(slide);
    }
    qDebug() << __FUNCTION__ << m_boxid;
}

void RecycleBox::append(int pos, const QString &sid)
{
    if (mPosList.contains(pos)) {
        if (mSlideMap.contains(pos) == false) {
            if (RtSampleManager::GetInstance()->containSlide(sid)) {
                auto slide = RtSampleManager::GetInstance()->getSlide(sid);
                slide->setRecycleBoxPos(pos > 10 ? pos - 52 : pos);
                slide->setRecycleBoxId(m_boxid);
                mSlideMap.insert(pos, slide);

                slide->setStatus(StainStatus::Recycle, 1);
                RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Done);

                /* record */
                auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
                if (db) {
                    QJsonObject setObj;
                    setObj.insert("slide_id", slide->slide_id());
                    setObj.insert("slide_uid", slide->slideUID());
                    setObj.insert("sample_id", slide->sampleID());
                    setObj.insert("sample_uid", slide->sampleUID());
                    db->updateRecord("slot", setObj, {{"slot_pos", pos}});
                }

            } else {
                qDebug() << "RecycleBox::append can NOT find slide:" << sid;
            }

            sendUIRecycleSlideInfo(pos, sid);
        }
    } else {
        qDebug() << __FUNCTION__ << "RecycleBox::append pos is NOT available. pos:" << pos << "List:" << mPosList;
    }
}

void RecycleBox::sendUIRecycleSlideInfo(int pos, const QString &slide_id)
{
    JPacket p(PacketType::Notification);
    p.module = "RecycleBox";
    p.api = "BoxInfo";

    if (pos >= mPosList.first()) {
        pos = pos - mPosList.first();
    }
    QJsonObject obj;
    obj.insert("pos", pos);
    obj.insert("box_id", m_boxid);

    if (RtSampleManager::GetInstance()->containSlide(slide_id)) {
        auto slide = RtSampleManager::GetInstance()->getSlide(slide_id);
        obj.insert("sample_uid", slide->sampleUID());
        obj.insert("slide_uid", slide->slideUID());
    }
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

bool RecycleBox::isFull()
{
    if (m_isClearMode == true) {
        if (mSlideMap.count() >= 10) {
            return true;
        }
    } else {
        if (mSlideMap.count() >= mPosList.count()) {
            return true;
        } else if (mSlideMap.count() >= m_maxCount) {
            return true;
        }
    }
    return false;
}

bool RecycleBox::isEmpty()
{
    return mSlideMap.isEmpty();
}

void RecycleBox::setMaxCount(int count)
{
    if (count > 0 && count <= mPosList.count()) {
        m_maxCount = count;
    }
}

void RecycleBox::eject()
{
    m_isEjected = true;
    qDebug() << m_boxid << "eject";
}

void RecycleBox::setClearMode()
{
    m_isClearMode = true;
    qDebug() << "Set Box:" << m_boxid << "Clear mode.";
}

int RecycleBox::getVacantPos()
{
    for (int i = 0; i < mPosList.count(); ++i) {
        int pos = mPosList.at(i);
        if (mSlideMap.contains(pos) == false) {
            return pos;
        }
    }
    return -1;
}

void RecycleBox::unloadAllSample()
{
    auto mgr = RtSampleManager::GetInstance();
    foreach (auto slide, mSlideMap) {
        mgr->removeSlideOne(slide);
    }
    mSlideMap.clear();
}

void RecycleBox::unloadBox()
{
    m_isUnloadedBox = true;

    /* record */
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
    if (db) {
        QJsonObject setObj;
        setObj.insert("slide_id", "");
        setObj.insert("slide_uid", "");
        setObj.insert("sample_id", "");
        setObj.insert("sample_uid", "");

        foreach (auto pos, mSlideMap.keys()) {
            db->updateRecord("slot", setObj, {{"slot_pos", pos}});
        }
    }

    JPacket p(PacketType::Notification);
    p.module = "RecycleBox";
    p.api = "UnloadBox";

    QJsonObject obj;
    obj.insert("box_id", m_boxid);
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RecycleBox::sendBoxToReader()
{
    JPacket p(PacketType::Notification);
    p.module = "RecycleBox";
    p.api = "SendBoxToReader";

    QJsonObject obj;
    obj.insert("box_id", m_boxid);
    obj.insert("box_info", getBoxInfo2());
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RecycleBox::sendBoxToManu()
{
    JPacket p(PacketType::Notification);
    p.module = "RecycleBox";
    p.api = "SendBoxToManu";

    QJsonObject obj;
    obj.insert("box_id", m_boxid);
    obj.insert("box_info", getBoxInfo2());
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RecycleBox::sendReaderSlideInfo()
{
    QJsonObject obj;
    obj.insert("box_id", m_boxid);
    obj.insert("box_info", getBoxInfo());

    if (FMessageCenter::GetInstance()->isReaderConnected()) {
        JPacket p(PacketType::Request);
        p.module = "Import";
        p.api = "AddNewSlideBox";
        p.paramsValue = obj;

        FMessageCenter::GetInstance()->sendReaderMessage(p);
    }
}

bool RecycleBox::containsStainSlide()
{
    foreach (auto slide, mSlideMap) {
        if (slide->isStainEnable() == true) {
            return true;
        }
    }
    qDebug() << m_boxid << "containsStainSlide? false";
    return false;
}

bool RecycleBox::allStainSlide()
{
    foreach (auto slide, mSlideMap) {
        if (slide->isStainEnable() == false) {
            qDebug() << "allStainSlide()?" << slide->slide_id() << "isStainEnable=false";
            return false;
        } else if (slide->isCancelled() == true) {
            qDebug() << "allStainSlide()?" << slide->slide_id() << "isCancelled=true";
            return false;
        }
    }
    return true;
}

void RecycleBox::setAllSlideDone()
{
    auto mgr = RtSampleManager::GetInstance();
    foreach (auto slide, mSlideMap) {
        mgr->sendUISlideStatus(slide, SlideProcessState::Done);
    }
}

QJsonArray RecycleBox::getBoxInfo()
{
    QJsonArray arr;
    for (int i = 0; i < mPosList.count(); ++i) {
        QJsonObject obj;
        obj.insert("pos", i + 1);

        int pos = mPosList.at(i);
        if (mSlideMap.contains(pos)) {
            auto slide = mSlideMap.value(pos);
            if (slide) {
                QJsonObject slideObj;
                slideObj.insert("QRcode", slide->qrcode());
                slideObj.insert("sample_id", slide->sampleID());
                auto sample = slide->sample();
                if (sample) {
                    slideObj.insert("order_uid", sample->order_uid());
                }
                obj.insert("slide", slideObj);
            }
        }
        arr.append(obj);
    }
    return arr;
}

QJsonArray RecycleBox::getBoxInfo2()
{
    QJsonArray arr;
    for (int i = 0; i < mPosList.count(); ++i) {
        int pos = mPosList.at(i);
        if (mSlideMap.contains(pos)) {
            QJsonObject obj;
            obj.insert("pos", i);

            auto slide = mSlideMap.value(pos);
            if (slide) {
                QJsonObject slideObj;
                slideObj.insert("QRcode", slide->qrcode());
                slideObj.insert("sample_id", slide->sampleID());
                slideObj.insert("slide_uid", slide->slideUID());
                auto sample = slide->sample();
                if (sample) {
                    slideObj.insert("sample_uid", slide->sampleUID());
                    slideObj.insert("order_uid", sample->order_uid());
                }
                obj.insert("slide", slideObj);
            }
            arr.append(obj);
        }
    }
    return arr;
}
