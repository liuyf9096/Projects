#include "rt_rack.h"
#include "sample/rt_sample_manager.h"

#include <QDebug>

RtRack::RtRack(const QString &rackid, QObject *parent)
    : QObject(parent)
    , m_rackId(rackid)
{
    m_isCanceled = false;
    m_isAborted = false;
    m_coord = 0;

    for (int i = 0; i < 10; ++i) {
        m_scanPosMap.insert(i, UnScaned);
    }
    qDebug() << "NEW Rack:" << rackid;
}

RtRack::~RtRack()
{
    auto mgr = RtSampleManager::GetInstance();
    foreach (auto sample, mSampleMap) {
        mgr->removeSampleOne(sample->sid());
    }

    mSampleMap.clear();
    qDebug() << __FUNCTION__ << m_rackId;
}

void RtRack::setCanceled()
{
    m_isCanceled = true;

    abort();
    qDebug() << m_rackId << "cancelled.";
}

QSharedPointer<RtSample> RtRack::addNewSample(int pos)
{
    auto sample = RtSampleManager::GetInstance()->NewSample();
    sample->setRackInfo(m_rackId, pos);
    mSampleMap.insert(pos, sample);

    setSampleExist(pos, true);
    return sample;
}

QSharedPointer<RtSample> RtRack::sampleAt(int index)
{
    if (mSampleMap.contains(index)) {
        return mSampleMap.value(index);
    }
    return nullptr;
}

void RtRack::unloadAllSample()
{
//    foreach (auto sample, mSampleMap) {

//    }
}

void RtRack::setCoord(int coord)
{
    m_coord = coord;
    foreach (auto sample, mSampleMap) {
        sample->setCoord(coord);
    }
}

//! [Scan]
void RtRack::setSampleExist(int pos, bool exist)
{
    if (m_scanPosMap.contains(pos)) {
        if (exist) {
            m_scanPosMap.insert(pos, Exist);
        } else {
            m_scanPosMap.insert(pos, Vacant);
        }
    }
}

int RtRack::getNextScanPos()
{
    if (m_isAborted == true) {
        return -1;
    }

    for (int i = 0; i < m_scanPosMap.count(); ++i) {
        PoseStatus status = m_scanPosMap.value(i);
        if (status == UnScaned) {
            return i;
        }
    }
    return -1;
}

void RtRack::abort()
{
    m_isAborted = true;

    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getStatus(SampleStatus::Sending_To_Test) == 0) {
                sample->setStatus(SampleStatus::Abort, 1);
                sample->setCanceled(true);
            }
        }
    }
    qDebug().noquote() << QString("Rack:%1 abort.").arg(m_rackId);
}

QList<QSharedPointer<RtSample>> RtRack::getUnSendToTestList()
{
    QList<QSharedPointer<RtSample>> list;
    if (m_isAborted == false) {
        for (int i = 0; i < 10; ++i) {
            if (mSampleMap.contains(i)) {
                auto sample = mSampleMap.value(i);
                if (!sample->sampleUID().isEmpty()) {
                    if (sample->getStatus(SampleStatus::Sending_To_Test) == 0
                            && sample->getStatus(SampleStatus::Abort) == 0) {
                        list.append(sample);
                    }
                }
            }
        }
    }
    return list;
}

QSharedPointer<RtSample> RtRack::getUnRecycleSample()
{
    foreach (auto sample, mSampleMap) {
        if (sample->getStatus(SampleStatus::Sampling) == 1) {
            if (sample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
                return sample;
            }
        }
    }
    return nullptr;
}

int RtRack::getUnFinishedPos()
{
    if (isAllScaned() == false) {
        return -1;
    }

    int i = 0;
    for (; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getStatus(SampleStatus::Abort) == 0) {
                if (sample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
                    break;
                }
            }
        }
    }
    return i;
}

bool RtRack::isAllScaned()
{
    if (getNextScanPos() >= 0) {
        return false;
    }
    return true;
}

bool RtRack::isAllRecycleFinished()
{
    if (isAllScaned() == true) {
        return false;
    }

    foreach (auto sample, mSampleMap) {
        if (sample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
            return false;
        }
    }
    return false;
}

bool RtRack::isAllTestFinished()
{
    if (isAllScaned() == false) {
        return false;
    }

    foreach (auto sample, mSampleMap) {
        if (sample->getStatus(SampleStatus::Abort) == 0) {
            if (sample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
                return false;
            }
        }
    }
    return true;
}
