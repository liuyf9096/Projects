#include "rt_rack.h"
#include "sample/rt_sample_manager.h"

#include <QDebug>

RtRack::RtRack(const QString &rackid, quint64 index, QObject *parent)
    : QObject(parent)
    , m_rackId(rackid)
    , m_index(index)
    , m_isCanceled(false)
{
    for (int i = 0; i < 10; ++i) {
        m_scanPosMap.insert(i, UnScaned);
    }
    qDebug() << "NEW Rack:" << rackid;
}

RtRack::~RtRack()
{
    mSampleMap.clear();
    qDebug() << __FUNCTION__ << m_rackId;
}

void RtRack::setCanceled(bool isCanceled)
{
    m_isCanceled = isCanceled;

    for (auto sample : qAsConst(mSampleMap)) {
        sample->setCanceled(true);
    }
    qDebug() << m_rackId << "canceled.";
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

QSharedPointer<RtSample> RtRack::lastSample()
{
    if (mSampleMap.isEmpty() == false) {
        return samples().last();
    }
    return nullptr;
}

QList<QSharedPointer<RtSample>> RtRack::samples()
{
    QList<QSharedPointer<RtSample> > list;
    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            list.append(mSampleMap.value(i));
        }
    }
    return list;
}

bool RtRack::isEmpty()
{
    return mSampleMap.isEmpty();
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

QList<QSharedPointer<RtSample>> RtRack::getUnSendToTestList()
{
    QList<QSharedPointer<RtSample>> list;
    for (int i = 0; i < 10; ++i) {
        if (mSampleMap.contains(i)) {
            auto sample = mSampleMap.value(i);
            if (sample->test_uid().isEmpty() == false) {
                if (sample->getState(SampleStatus::Send_To_Test) == 0) {
                    list.append(sample);
                }
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

QSharedPointer<RtSample> RtRack::getSamplingFinishedSample()
{
    for (auto sample : qAsConst(mSampleMap)) {
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

    for (auto sample : qAsConst(mSampleMap)) {
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

    for (auto sample : qAsConst(mSampleMap)) {
        if (sample->getState(SampleStatus::Test_Finished) == 0) {
            return false;
        }
    }
    return true;
}

bool RtRack::isAllFinished()
{
    if (isAllScaned() == false) {
        return false;
    } else if (mSampleMap.isEmpty()) {
        return true;
    }

    for (auto sample : qAsConst(mSampleMap)) {
        if (sample->getState(SampleStatus::Send_To_Test) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Test_Finished) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Recycle_To_Rack) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Review_Sample) == 0) {
            return false;
        } else if (sample->getState(SampleStatus::Need_Smear) == 1) {
            if (sample->getState(SampleStatus::Smear_Done) == 0) {
                return false;
            }
        }
    }
    return true;
}

bool RtRack::isNeedReview()
{
    if (isAllTestFinished()) {
        for (auto sample : qAsConst(mSampleMap)) {
            if (sample->getState(SampleStatus::Review_Sample) == 0) {
                return true;
            }
        }
    }
    return false;
}

void RtRack::setAllSampleReviewed()
{
    for (auto sample : qAsConst(mSampleMap)) {
        sample->setState(SampleStatus::Review_Sample, 1);
    }
}

void RtRack::setAllSmearProgram()
{
    for (auto sample : qAsConst(mSampleMap)) {
        sample->setState(SampleStatus::Need_Smear, 1);
        sample->setProgramList({"smear"});
        qDebug() << "set all samples need smear." << sample->sid();
    }
}

void RtRack::setTestHalfSmearProgram()
{
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
