#include "stainonly_base.h"
#include "sample/rt_sample_manager.h"
//#include "pool_manager.h"

StainOnlyBase::StainOnlyBase(const QString &mid, const QString &userid, int from, int to, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    m_isHandling = false;

    for (int i = from; i <= to; ++i) {
        PosList.append(i);
    }
    qInfo().noquote() << QString("[%1] :").arg(mUserId) << PosList;
}

bool StainOnlyBase::isEmpty()
{
    return m_posSlideMap.isEmpty();
}

bool StainOnlyBase::loadSlides(quint64 id, const QJsonArray &arr)
{
    if (m_isHandling == false && m_posSlideMap.isEmpty()) {
        m_isHandling = true;
        m_id = id;

        for (int i = 0; i < arr.count(); ++i)
        {
            QJsonObject obj = arr.at(i).toObject();
            int pos = obj.value("pos").toInt();
            QString sample_uid = obj.value("sample_uid").toString();
            QString slide_uid = obj.value("slide_uid").toString();

            if (sample_uid.isEmpty() == false && PosList.contains(pos) && m_posSlideMap.contains(pos) == false) {
                QString sid = "SO_" + sample_uid;

                auto slide = RtSampleManager::GetInstance()->NewSampleSlide(sid);
                slide->setSlideUID(slide_uid);
                slide->setSampleUID(sample_uid);

                slide->setStainEnable(true);
                slide->setSmearEnable(false);
                slide->setPrintEnable(false);

                slide->setSendRequest(false);
                slide->setStainPos(pos);

                RtSampleManager::GetInstance()->setSlideStainProcess(slide);
                RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Ready);
                m_posSlideMap.insert(pos, slide);
            } else {
                qDebug() << mUserId << "stainOnly pos error." << m_posSlideMap << obj;
            }
        }

//        dev->stain()->setCheckSensorEnable(mModuleId, true);

        setLightRun();

        qDebug() << mUserId << "load StainOnly slides" << m_posSlideMap;
        return true;
    }
    return false;
}

void StainOnlyBase::unloadAllSlides()
{
    qDebug() << mUserId << "unload All Slides";

    foreach (auto slide, m_posSlideMap) {
        slide->setStatus(SmearStatus::Cancel, 1);
        RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Canceled);
        RtSampleManager::GetInstance()->removeSlideOne(slide->slide_id());
    }
    m_posSlideMap.clear();

    if (m_isHandling) {
        m_isHandling = false;

        JPacket p(PacketType::Result, m_id);
        p.resValue = true;
        sendUIMessage(p);
    }

    dev->stain()->setCheckSensorEnable(mModuleId, false);
}

bool StainOnlyBase::hasRequest()
{
    if (!m_posSlideMap.isEmpty()) {
        foreach (int pos, PosList) {
            auto slide = m_posSlideMap.value(pos);
            if (slide && slide->isSendRequest() == false) {
                return true;
            }
        }
    }
    return false;
}

void StainOnlyBase::sendRequest()
{
    if (!m_posSlideMap.isEmpty()) {
        auto slide = m_posSlideMap.last();
        if (slide->isSendRequest() == false) {
            slide->setSendRequest(true);

            RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Processing);

            emit onSendRequest_singal(mUserId, slide->stainPos(), slide->slide_id());
        }
    }
}

bool StainOnlyBase::takeOutSample(const QString &sid, int pos)
{
    if (m_posSlideMap.value(pos)->slide_id() == sid) {
        m_posSlideMap.remove(pos);
        emit onTakeOutOfPos_signal(pos, sid);

        if (m_posSlideMap.isEmpty()) {
            m_isHandling = false;
            setLightStop();

            JPacket p(PacketType::Result, m_id);
            p.resValue = true;
            sendUIMessage(p);

            dev->stain()->setCheckSensorEnable(mModuleId, false);
        }
        return true;
    }
    return false;
}
