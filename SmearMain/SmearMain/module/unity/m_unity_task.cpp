#include "m_unity_task.h"
#include "smear/m_sampling.h"
#include "module_manager.h"
#include "sample/rt_sample_manager.h"

MUnityTask::MUnityTask(QObject *parent)
    : DModuleBase("unity", "unity", parent)
{
    mLabel = "unity";
    state_init();

    mRotateTimer = new QTimer(this);
    mRotateTimer->setInterval(100);
    connect(mRotateTimer, &QTimer::timeout,
            this, &MUnityTask::rotateTubeTimer_slot);

    mTakeUpTimer = new QTimer(this);
    mTakeUpTimer->setInterval(100);
    connect(mTakeUpTimer, &QTimer::timeout,
            this, &MUnityTask::takeupTimer_slot);

    mRecycleTimer = new QTimer(this);
    mRecycleTimer->setInterval(100);
    connect(mRecycleTimer, &QTimer::timeout,
            this, &MUnityTask::recycleTimer_slot);

    mCheckTimer = new QTimer(this);
    mCheckTimer->setInterval(100);
    connect(mCheckTimer, &QTimer::timeout,
            this, &MUnityTask::checkTimer_slot);
}

void MUnityTask::state_init()
{
    m_receiveSample = nullptr;
    m_recycleSample = nullptr;

    m_rotateId = 0;
    m_isRotateFinished = true;
    s_rotate = RotateState::Idle;

    m_takeupId = 0;
    m_isTakeupFinished = true;
    s_takeup = TakeUpState::Idle;

    m_recycleId = 0;
    m_isRecycleFinished = true;
    s_recycle = RecycleState::Idle;
}

/* "IsReceiveNewSampleAble" */
void MUnityTask::checkAvailable(quint64 id, const QJsonObject &obj)
{
    Q_UNUSED(obj)

    if (mManager->isRunning() == true && mSampling->isReceivable()) {
        sendResultMessage(id, true);
    } else {
        sendResultMessage(id, false);
    }
}

bool MUnityTask::takeUpNewSample(quint64 id, const QJsonObject &obj)
{
    bool ok = false;

    if (obj.contains("order") == false) {
        qWarning() << "sample order is MISSING.";
        return false;
    }

    QString test_uid = obj.value("test_uid").toString();
    QJsonObject orderObj = obj.value("order").toObject();
    if (orderObj.isEmpty()) {
        qWarning() << "order obj is Empty.";
        return false;
    }
    if (test_uid.isEmpty()) {
        qWarning() << "test_uid is MISSING.";
        return false;
    }

    if (m_isTakeupFinished == true) {
        m_isTakeupFinished = false;
    } else {
        sendErrorMessage(id, "Take up Sample Error.");
        return false;
    }

    auto sampleManager = RtSampleManager::GetInstance();
    auto sample = sampleManager->NewSample();
    sample->setPrintEnable(sampleManager->isAutoPrint());
    sample->setSmearEnable(sampleManager->isAutoSmear());
    sample->setStainEnable(sampleManager->isAutoStain());
    sample->setEmergency(false);

    sample->setOrderInfo(orderObj);
    sample->setDefaultPrintInfo();

    QString bloodType = orderObj.value("blood_sample_pattern").toString();
    if (bloodType.contains("wholeblood", Qt::CaseInsensitive)) {
        sample->setMiniBlood(false);
    }

    if (mSampling->isReceivable()) {
        m_receiveSample = sample;
        m_takeupId = id;
        s_takeup = TakeUpState::Idle;

        mTakeUpTimer->start();
        RtSampleManager::GetInstance()->requestSampleOrder(sample);
    } else {
        sampleManager->removeSampleOne(sample->sid());
        m_isTakeupFinished = true;
        sendErrorMessage(id, "Take up Sample Error.");
    }
    return ok;
}

bool MUnityTask::recycleSample(quint64 id, const QJsonObject &obj)
{
    bool ok = false;
    if (m_isRecycleFinished == true) {
        m_isRecycleFinished = false;

        QString test_uid = obj.value("test_uid").toString();
        if (m_recycleSample->test_uid() == test_uid) {
            ok = mSampling->sendRecycleSample(m_recycleSample);
            if (ok) {
                m_recycleId = id;
                s_recycle = RecycleState::Idle;
                mRecycleTimer->start();
            }
        }
        if (ok == false) {
            m_isRecycleFinished = true;
            sendErrorMessage(id, "Recycle Sample Error.");
        }
    }
    return ok;
}

void MUnityTask::rotateTube(quint64 id)
{
    if (m_isRotateFinished == true) {
        m_isRotateFinished = false;
        m_rotateId = id;
        s_rotate = RotateState::Idle;
        mRotateTimer->start();
    }
}

void MUnityTask::rotateTubeTimer_slot()
{
    switch (s_rotate) {
    case RotateState::Idle:
        if (m_isRotateFinished == false) {
            s_rotate = RotateState::Rotate_Tube;
        }
        break;
    case RotateState::Rotate_Tube:
    {
        bool ok = dev->sample()->cmd_RotateTube();
        if (ok) {
            s_rotate = RotateState::WaitF_Rotate_Done;
        }
        break;
    }
    case RotateState::WaitF_Rotate_Done:
        if (dev->sample()->getFuncResult("RotateTube") == Func_Done) {
            m_isRotateFinished = true;
            sendResultMessage(m_rotateId);
            s_rotate = RotateState::Finish;
        } else if (dev->sample()->getFuncResult("RotateTube") == Func_Fail) {
            sendErrorMessage(m_rotateId, "rotate");
            s_rotate = RotateState::Finish;
        }
        break;
    case RotateState::Finish:
        mRotateTimer->stop();
        s_rotate = RotateState::Idle;
        break;
    }
}

void MUnityTask::takeupTimer_slot()
{
    switch (s_takeup) {
    case TakeUpState::Idle:
        if (m_isTakeupFinished == false) {
            if (m_receiveSample->sampleUID().isEmpty() == false) {
                mSampling->receiveNewSample(m_receiveSample);
                s_takeup = TakeUpState::WaitF_TakeUp_Done;
            }
        }
        break;

    case TakeUpState::WaitF_TakeUp_Done:
        if (mSampling->isReceiveFinished()) {
            m_isTakeupFinished = true;
            sendResultMessage(m_takeupId);
            s_takeup = TakeUpState::Finish;
        }
        break;

    case TakeUpState::Finish:
        if (m_recycleSample == nullptr) {
            m_recycleSample = m_receiveSample;
            m_receiveSample = nullptr;

            mTakeUpTimer->stop();
            mCheckTimer->start(100);
            s_takeup = TakeUpState::Idle;
        }
        break;
    }
}

void MUnityTask::recycleTimer_slot()
{
    switch (s_recycle) {
    case RecycleState::Idle:
        if (m_isRecycleFinished == false) {
            s_recycle = RecycleState::WaitF_Recycle_Done;
        }
        break;
    case RecycleState::Recycle_Sample:
        s_recycle = RecycleState::WaitF_Recycle_Done;
        break;
    case RecycleState::WaitF_Recycle_Done:
        if (mSampling->isRecycleFinished()) {
            m_isRecycleFinished = true;
            m_recycleSample = nullptr;
            sendResultMessage(m_recycleId);
            s_recycle = RecycleState::Finish;
        }
        break;
    case RecycleState::Finish:
        mRecycleTimer->stop();
        s_recycle = RecycleState::Idle;
        break;
    }
}

void MUnityTask::checkTimer_slot()
{
    if (m_recycleSample == nullptr) {
        qDebug() << "SamplingFinished check stop.";
        mCheckTimer->stop();
    } else {
        if (m_recycleSample->getStatus(SampleStatus::Sampling) == 1) {
            JPacket p(PacketType::Notification);
            p.module = "Notification";
            p.api = "Sample";

            QJsonObject obj;
            obj.insert("test_uid", m_recycleSample->test_uid());
            obj.insert("status", "SamplingFinished");
            p.paramsValue = obj;

            FMessageCenter::GetInstance()->sendUnityMessage(p);
            mCheckTimer->setInterval(10 * 1000);
        }
    }
}

void MUnityTask::sendResultMessage(quint64 id, const QJsonValue &res)
{
    JPacket p(PacketType::Result, id);
    p.resValue = res;
    FMessageCenter::GetInstance()->sendUnityMessage(p);
}

void MUnityTask::sendErrorMessage(quint64 id, const QString &msg)
{
    JPacket p(PacketType::Error, id);
    p.errorCode = 90;
    p.errorMessage = QString("command Error. api:%1").arg(msg);
    FMessageCenter::GetInstance()->sendUnityMessage(p);
}
