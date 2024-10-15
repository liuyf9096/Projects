#include "rt_rack.h"
#include "sample/rt_sample_manager.h"

#include <QDateTime>
#include <QDebug>

RtRack::RtRack(const QString &rackid, quint64 index, QObject *parent)
    : QObject(parent)
    , m_rackId(rackid)
    , m_index(index)
    , m_isCanceled(false)
{
    m_coord = 0;
    m_reviewCount = 0;
    m_isSmearMode = false;

    for (int i = 0; i < 10; ++i) {
        m_scanPosMap.insert(i, UnScaned);
    }
    qDebug() << "NEW Rack:" << rackid;
}

RtRack::~RtRack()
{
    foreach (auto sample, mSampleMap) {
        RtSampleManager::GetInstance()->removeSampleOne(sample);
    }
    mSampleMap.clear();
    qDebug() << __FUNCTION__ << m_rackId;
}

void RtRack::setCanceled()
{
    m_isCanceled = true;

    foreach (auto sample, mSampleMap) {
        sample->setCanceled(true);
    }
    qDebug() << m_rackId << "cancelled.";
    RtSampleManager::GetInstance()->sendOperationInfo("Cancelled");
}

void RtRack::setBarcode(const QString &barcode)
{
    m_barcode = barcode;
    qDebug() << m_rackId << __FUNCTION__ << barcode;
}

void RtRack::setSmearMode()
{
    m_isSmearMode = true;
    qDebug() << m_rackId << "setSmearMode";
}

void RtRack::setReviewMode(bool isDone, const QString &mode)
{
    foreach (auto sample, mSampleMap) {
        sample->setRetestMode(isDone, mode);
    }
}

QSharedPointer<RtSample> RtRack::addNewSample(int pos)
{
    auto sample = RtSampleManager::GetInstance()->NewSample();
    sample->setRackInfo(m_rackId, pos);
    sample->setCoord(m_coord + pos);
    mSampleMap.insert(pos, sample);

    setSampleExist(pos, true);

    if (m_isSmearMode == true) {
        sample->setSmearMode();
    }
    return sample;
}

QSharedPointer<RtSample> RtRack::sampleAt(int index)
{
    if (mSampleMap.contains(index)) {
        return mSampleMap.value(index);
    }
    return nullptr;
}

void RtRack::sendSamplePath(const QString &node_id)
{
    foreach (auto sample, mSampleMap) {
        RtSampleManager::GetInstance()->sendSamplePath(sample, node_id);
    }
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
    for (int i = 0; i < m_scanPosMap.count(); ++i) {
        ScanStatus status = m_scanPosMap.value(i);
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
            if (sample->getState(SampleStatus::Send_To_Test) == 0) {
                sample->setState(SampleStatus::Canceled, 1);
            }
        }
    }
    qDebug().noquote() << QString("Rack:%1 abort.").arg(m_rackId);
}

QList<QSharedPointer<RtSample>> RtRack::getUnSendToTestSampleList()
{
    QList<QSharedPointer<RtSample>> list;
    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getState(SampleStatus::Send_To_Test) == 0) {
                list.append(sample);
            }
        }
    }
    return list;
}

QList<QSharedPointer<RtSample> > RtRack::getUnSendToSmearList()
{
    QList<QSharedPointer<RtSample>> list;
    if (m_isCanceled) {
        return list;
    }

    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getState(SampleStatus::Send_To_Smear) == 0
                    && sample->getState(SampleStatus::Need_Smear) == 1) {
                list.append(sample);
            }
        }
    }
    return list;
}

QSharedPointer<RtSample> RtRack::getUnRecycleSample()
{
    foreach (auto sample, mSampleMap) {
        if (sample->getState(SampleStatus::SamplingFinished) == 1) {
            if (sample->getState(SampleStatus::Recycle_To_Rack) == 0) {
                return sample;
            }
        }
    }
    return nullptr;
}

int RtRack::getUnTestedPos()
{
    if (m_isCanceled == true) {
        return -1;
    }
    if (isAllScaned() == false) {
        return -1;
    }

    int i = 0;
    for (; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getState(SampleStatus::Test_Finished) == 0) {
                break;
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

bool RtRack::isAllSendToTest()
{
    if (isAllScaned() == true) {
        return false;
    }

    foreach (auto sample, mSampleMap) {
        if (sample->getState(SampleStatus::Send_To_Test) == 0) {
            return false;
        }
    }
    return true;
}

bool RtRack::isAllTestFinished()
{
    if (isAllScaned() == false) {
        return false;
    }

    foreach (auto sample, mSampleMap) {
        if (sample->getState(SampleStatus::Test_Finished) == 0) {
            return false;
        }
    }
    return true;
}

bool RtRack::isAllReviewed()
{
    foreach (auto sample, mSampleMap) {
        if (sample->getState(SampleStatus::Review_Sample) == 0) {
            return false;
        }
    }
    return true;
}

bool RtRack::isAllReviewedDone()
{
    if (isAllScaned() == false) {
        return false;
    } else {
        foreach (auto sample, mSampleMap) {
            if (sample->getState(SampleStatus::Review_Done) == 0) {
                if (sample->getState(SampleStatus::Need_Smear) == 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

int RtRack::minReviewDonePos()
{
    if (m_isCanceled == true) {
        return 10;
    }
    if (isAllScaned() == false) {
        return -1;
    }

    int i = 0;
    for (; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getState(SampleStatus::Review_Sample) != 1) {
                break;
            }
        }
    }
    return i;
}

bool RtRack::isAllFinished()
{
    if (isAllScaned() == false) {
        return false;
    } else if (mSampleMap.isEmpty()) {
        return true;
    }

    foreach (auto sample, mSampleMap) {
        if (sample->getState(SampleStatus::Send_To_Test) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Test_Finished) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Recycle_To_Rack) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Review_Sample) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Review_Done) == 0) {
            return false;
        }
    }
    return true;
}

bool RtRack::isNeedReview()
{
    if (isAllTestFinished()) {
        foreach (auto sample, mSampleMap) {
            if (sample->getState(SampleStatus::Review_Sample) == 0) {
                return true;
            }
        }
    }
    return false;
}

void RtRack::doReview(bool isClassify, bool isSmear)
{
    qDebug() << __FUNCTION__ << "BloodTestEn:" << isClassify << "SmearEn:" << isSmear;

    foreach (auto sample, mSampleMap) {
        sample->doReviewMode(isClassify, isSmear);
    }
}

void RtRack::setAllSmearProgram()
{
    foreach (auto sample, mSampleMap) {
        sample->setRetestMode(false, "smear");
    }
}

void RtRack::clearSampleProgram()
{
    foreach (auto sample, mSampleMap) {
        sample->setProgramList({""});
    }
}

void RtRack::setTestHalfSmearProgram()
{
#if 0
    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (i % 2 == 0) {
                sample->setState(SampleStatus::Need_Smear, 1);
                sample->setProgramList({"smear"});
                qDebug() << "for test set sample" << sample->sid();
            }
        }
    }
#endif
}

bool RtRack::isSampleSmearMode()
{
    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->getState(SampleStatus::Need_Smear) == 1) {
                return true;
            }
        }
    }
    return false;
}
