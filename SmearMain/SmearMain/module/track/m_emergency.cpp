#include "m_emergency.h"
#include "messagecenter/f_message_center.h"
#include "smear/m_sampling.h"
#include "f_common.h"
#include "cart/carts_manager.h"

MEmergency::MEmergency(const QString &mid, QObject *parent)
    : DModuleBase(mid, "Emergency", parent)
{
    mWaitSec = 60;
    m_isUnited = FCommon::GetInstance()->isUnited();

    state_init();

    mOperateTimer = new QTimer(this);
    mOperateTimer->setInterval(100);
    connect(mOperateTimer, &QTimer::timeout,
            this, &MEmergency::onClosetTimeout_slot);

    mEmergencyTimer = new QTimer(this);
    mEmergencyTimer->setInterval(100);
    connect(mEmergencyTimer, &QTimer::timeout,
            this, &MEmergency::onEmergencyProcess_slot);

    auto track = RtDeviceManager::GetInstance()->track();
    connect(track, &DTrack::onFunctionFinished_signal,
            this, &MEmergency::onFunctionFinished_slot);

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onUnityMessageResult_signal,
            this, &MEmergency::onUnityMessageResult_slot);
}

void MEmergency::state_init()
{
    m_isClosetOpen = false;
    m_isClosetFinished = true;
    m_closetid = 0;
    m_sample = nullptr;
    m_isClosetOpenFinished = true;
    m_isClosetCloseFinished = true;

    s_closet = OperateState::Idle;
    s_emerg = EmergState::Idle;
}

bool MEmergency::Closet_Open()
{
    qDebug() << "Open Closet";
    if (m_isUnited == true) {
        if (FMessageCenter::GetInstance()->isUnityConnected()) {
            if (m_isClosetOpenFinished == true) {
                m_isClosetOpenFinished = false;

                JPacket p("Unity", "Emergency", "ClosetOpen");
                FMessageCenter::GetInstance()->sendUnityMessage(p);
                return true;
            }
        } else {
            m_isClosetOpenFinished = true;
            return true;
        }
    } else {
        dev->track()->cmd_Emergency_Open();
        return true;
    }
    return false;
}

bool MEmergency::Closet_Close()
{
    qDebug() << "Close Closet";
    if (m_isUnited == true) {
        if (FMessageCenter::GetInstance()->isUnityConnected()) {
            if (m_isClosetCloseFinished == true) {
                m_isClosetCloseFinished = false;

                JPacket p("Unity", "Emergency", "ClosetClose");
                FMessageCenter::GetInstance()->sendUnityMessage(p);
                return true;
            }
        }
    } else {
        dev->track()->cmd_Emergency_Close();
        return true;
    }
    return false;
}

bool MEmergency::isClosetOpenDone()
{
    if (m_isUnited == true) {
        return m_isClosetOpenFinished;
    } else {
        return dev->track()->isFuncDone("Emergency_Open");
    }
    return false;
}

bool MEmergency::isClosetCloseDone()
{
    if (m_isUnited == true) {
        return m_isClosetCloseFinished;
    } else {
        return dev->track()->isFuncDone("Emergency_Close");
    }
    return false;
}

void MEmergency::handleRequest(const JPacket &packet)
{
    if (packet.module == "Emergency") {
        auto obj = packet.paramsValue.toObject();
        if (packet.api == "EmergencyCloset") {
            entranceCloset(packet.id, obj);
        } else if (packet.api == "Start") {
            startEmergency(packet.id, obj);
        }
    }
}

void MEmergency::entranceCloset(quint64 id, const QJsonObject &obj)
{
    if (m_isClosetFinished == true && obj.contains("on")) {
        m_isClosetFinished = false;
        m_isOn = obj.value("on").toBool();
        s_closet = OperateState::Idle;
        m_closetid = id;
        mOperateTimer->start();
    } else {
        qDebug() << __FUNCTION__ << "Error: m_isClosetFinished:" << m_isClosetFinished;
    }
}

void MEmergency::startEmergency(quint64 id, const QJsonObject &obj)
{
    if (m_sample == nullptr) {
        m_sample = RtSampleManager::GetInstance()->NewSample();
        setEmergencySample(m_sample, obj);

        m_emergId = id;
        s_emerg = EmergState::Idle;

        if (!mEmergencyTimer->isActive()) {
            mEmergencyTimer->start();
        }
    }
}

void MEmergency::setEmergencySample(QSharedPointer<RtSample> sample, const QJsonObject &obj)
{
    sample->setEmergency(true);

    if (obj.contains("sample_uid")) {
        QString sample_uid = obj.value("sample_uid").toString();
        sample->setSampleUID(sample_uid);
    }
    if (obj.contains("sample_id")) {
        QString sample_id = obj.value("sample_id").toString();
        sample->setSampleID(sample_id);
    }
    if (obj.contains("order_uid")) {
        QString order_uid = obj.value("order_uid").toString();
        sample->setOrder_uid(order_uid);
    }

    if (obj.contains("sample_type")) {
        QString sample_type = obj.value("sample_type").toString();
        if (sample_type.contains("wholeblood", Qt::CaseInsensitive)) {
            sample->setCapped(true);
            sample->setMiniBlood(false);
        } else if (sample_type.contains("miniblood", Qt::CaseInsensitive)) {
            sample->setCapped(false);
            sample->setMiniBlood(true);
        }
    }

    if (obj.contains("isRet")) {
        bool isRet = obj.value("isRet").toBool();
        sample->setRet(isRet);
    }

    if (obj.contains("smear")) {
        QJsonObject smearObj = obj.value("smear").toObject();
        int smearCount = smearObj.value("count").toInt();
        bool isPrint = smearObj.value("isPrint").toBool();
        bool isSmear = smearObj.value("isSmear").toBool();
        bool isStain = smearObj.value("isStain").toBool();
        double hct = smearObj.value("hct").toDouble();
        sample->setHtc(hct);
        sample->setSmearCount(smearCount);
        sample->setPrintEnable(isPrint);
        sample->setSmearEnable(isSmear);
        sample->setStainEnable(isStain);
    }

    if (obj.contains("slides")) {
        QJsonArray arr = obj.value("slides").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject slideObj = arr.at(i).toObject();
            QString slide_uid = slideObj.value("slide_uid").toString();
            QJsonObject printObj = slideObj.value("print").toObject();

            QString slide_id = QString("%1-%2").arg(sample->sid()).arg(i + 1);
            auto slide = RtSampleManager::GetInstance()->NewSampleSlide(slide_id);
            RtSampleManager::GetInstance()->syncSlide(slide, sample);

            slide->setSlideUID(slide_uid);
            slide->setPrintInfo(printObj);

            sample->slides.append(slide);
        }
    }
}

void MEmergency::reset()
{
    state_init();
}

void MEmergency::onClosetTimeout_slot()
{
    if (m_isOn == true) {
        switch (s_closet) {
        case OperateState::Idle:
            if (m_isClosetFinished == false) {
                if (m_isUnited) {
                    if (FMessageCenter::GetInstance()->isUnityConnected()) {
                        s_closet = OperateState::Operate_Closet;
                    } else {
                        s_closet = OperateState::United_Finish;
                    }
                } else {
                    s_closet = OperateState::Operate_Closet;
                }
            }
            break;

        case OperateState::Operate_Closet:
            if (m_isUnited) {
                if (FMessageCenter::GetInstance()->isUnityConnected()) {
                    if (m_isClosetOpenFinished == true) {
                        m_isClosetOpenFinished = false;

                        JPacket p("Unity", "Emergency", "ClosetOpen");
                        FMessageCenter::GetInstance()->sendUnityMessage(p);
                        s_closet = OperateState::WaitF_Operate_Closet_Done;
                    }
                } else {
                    s_closet = OperateState::United_Finish;
                }
            } else {
                dev->track()->cmd_Emergency_Open();
                s_closet = OperateState::WaitF_Operate_Closet_Done;
            }
            break;
        case OperateState::WaitF_Operate_Closet_Done:
            if (m_isUnited) {
                if (FMessageCenter::GetInstance()->isUnityConnected()) {
                    if (m_isClosetOpenFinished == true) {
                        s_closet = OperateState::United_Finish;
                    }
                } else {
                    s_closet = OperateState::United_Finish;
                }
            } else if (dev->track()->isFuncDone("Emergency_Open")) {
                s_closet = OperateState::Finish;
            }
            break;

        case OperateState::Finish:
            mOperateTimer->stop();
            m_isClosetFinished = true;
            s_closet = OperateState::Idle;
            break;

        case OperateState::United_Finish:
        {
            JPacket p(PacketType::Result, m_closetid);
            p.resValue = true;
            sendUIMessage(p);
        }
            mOperateTimer->stop();
            m_isClosetFinished = true;
            s_closet = OperateState::Idle;
            break;
        }
    } else {
        switch (s_closet) {
        case OperateState::Idle:
            if (m_isClosetFinished == false) {
                if (m_isUnited) {
                    if (FMessageCenter::GetInstance()->isUnityConnected()) {
                        s_closet = OperateState::Operate_Closet;
                    } else {
                        s_closet = OperateState::United_Finish;
                    }
                } else {
                    s_closet = OperateState::Operate_Closet;
                }
            }
            break;

        case OperateState::Operate_Closet:
            if (m_isUnited) {
                if (FMessageCenter::GetInstance()->isUnityConnected()) {
                    if (m_isClosetCloseFinished == true) {
                        m_isClosetCloseFinished = false;

                        JPacket p("Unity", "Emergency", "ClosetClose");
                        FMessageCenter::GetInstance()->sendUnityMessage(p);
                        s_closet = OperateState::WaitF_Operate_Closet_Done;
                    }
                } else {
                    s_closet = OperateState::United_Finish;
                }
            } else {
                dev->track()->cmd_Emergency_Close();
                s_closet = OperateState::WaitF_Operate_Closet_Done;
            }
            break;
        case OperateState::WaitF_Operate_Closet_Done:
            if (m_isUnited) {
                if (FMessageCenter::GetInstance()->isUnityConnected()) {
                    if (m_isClosetCloseFinished == true) {
                        s_closet = OperateState::United_Finish;
                    }
                } else {
                    s_closet = OperateState::United_Finish;
                }
            } else if (dev->track()->isFuncDone("Emergency_Close")) {
                s_closet = OperateState::Finish;
            }
            break;

        case OperateState::Finish:
            mOperateTimer->stop();
            m_isClosetFinished = true;
            s_closet = OperateState::Idle;
            break;

        case OperateState::United_Finish:
        {
            JPacket p(PacketType::Result, m_closetid);
            p.resValue = true;
            sendUIMessage(p);
        }
            mOperateTimer->stop();
            m_isClosetFinished = true;
            s_closet = OperateState::Idle;
            break;
        }
    }
}

void MEmergency::onEmergencyProcess_slot()
{
    switch (s_emerg) {
    case EmergState::Idle:
        if (m_sample != nullptr) {
            logProcess("Emergency", 1, "Idle", m_sample->sid());
            s_emerg = EmergState::CloseCloset;
        }
        break;

    case EmergState::CloseCloset:
        if (m_isUnited) {
            if (FMessageCenter::GetInstance()->isUnityConnected()) {
                if (m_isClosetCloseFinished == true) {
                    m_isClosetCloseFinished = false;

                    JPacket p("Unity", "Emergency", "ClosetClose");
                    FMessageCenter::GetInstance()->sendUnityMessage(p);
                    s_emerg = EmergState::WaitF_CloseCloset_Done;
                }
            } else {
                s_emerg = EmergState::Send_Sample_To_Test;
            }
        } else {
            dev->track()->cmd_Emergency_Close();
            s_emerg = EmergState::WaitF_CloseCloset_Done;
        }
        break;
    case EmergState::WaitF_CloseCloset_Done:
        if (m_isUnited) {
            if (FMessageCenter::GetInstance()->isUnityConnected()) {
                if (m_isClosetCloseFinished == true) {
                    s_emerg = EmergState::Send_Sample_To_Test;
                }
            } else {
                s_emerg = EmergState::Send_Sample_To_Test;
            }
        } else if (dev->track()->isFuncDone("Emergency_Close")) {
            s_emerg = EmergState::Send_Sample_To_Test;
        }
        break;

    case EmergState::Send_Sample_To_Test:
    {
        bool ok = mSampling->receiveEmergencySample(m_sample);
        if (ok) {
            logProcess("Emergency", 2, "Send Sample", m_sample->sid());
            m_sample->setStatus(SampleStatus::Sending_To_Test, 1);
            s_emerg = EmergState::WaitF_Test_Finished;
        }
    }
        break;
    case EmergState::WaitF_Test_Finished:
        if (m_sample->getStatus(SampleStatus::Test_Finished) == 1) {
            s_emerg = EmergState::Recycle_Sample;
        }
        break;

    case EmergState::Recycle_Sample:
    {
        bool ok = mSampling->sendRecycleSample(m_sample);
        if (ok) {
            logProcess("Emergency", 3, "Recycle Sample", m_sample->sid());
            s_emerg = EmergState::WaitF_Recycle_Sample_Done;
        }
    }
        break;
    case EmergState::WaitF_Recycle_Sample_Done:
        if (m_sample->getStatus(SampleStatus::Recycle_To_Rack) == 1) {
            s_emerg = EmergState::Open_Closet;
        }
        break;

    case EmergState::Open_Closet:
        if (m_isUnited) {
            if (FMessageCenter::GetInstance()->isUnityConnected()) {
                if (m_isClosetOpenFinished == true) {
                    m_isClosetOpenFinished = false;

                    JPacket p("Unity", "Emergency", "ClosetOpen");
                    FMessageCenter::GetInstance()->sendUnityMessage(p);
                    s_emerg = EmergState::WaitF_Open_Closet_Done;
                }
            } else {
                s_emerg = EmergState::United_Finish;
            }
        } else {
            dev->track()->cmd_Emergency_Open();
            s_emerg = EmergState::WaitF_Open_Closet_Done;
        }
        break;
    case EmergState::WaitF_Open_Closet_Done:
        if (m_isUnited) {
            if (FMessageCenter::GetInstance()->isUnityConnected()) {
                if (m_isClosetOpenFinished == true) {
                    s_emerg = EmergState::United_Finish;
                }
            } else {
                s_emerg = EmergState::United_Finish;
            }
        } else if (dev->track()->isFuncDone("Emergency_Open")) {
            s_emerg = EmergState::Finish;
        }
        break;

    case EmergState::Finish:
    {
        JPacket p(PacketType::Result, m_emergId);
        p.resValue = true;
        sendUIMessage(p);
    }

        logProcess("Emergency", 99, "Finished.");
        s_emerg = EmergState::Idle;

        RtSampleManager::GetInstance()->removeSampleOne(m_sample->sid());
        m_sample = nullptr;
        break;

    case EmergState::United_Finish:
    {
        JPacket p(PacketType::Result, m_emergId);
        p.resValue = true;
        sendUIMessage(p);
    }

        m_emergId = 0;
        RtSampleManager::GetInstance()->removeSampleOne(m_sample->sid());
        m_sample = nullptr;

        logProcess("Emergency", 99, "Finished.");
        s_emerg = EmergState::Idle;
        break;
    }
}

void MEmergency::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    Q_UNUSED(resValue)

    if (api == "Emergency_Open") {
        m_isClosetOpen = true;
    } else if (api == "Emergency_Close") {
        m_isClosetOpen = false;
    }
}

void MEmergency::onUnityMessageResult_slot(const JPacket &result, const JPacket &request)
{
    Q_UNUSED(result)

    if (request.api == "ClosetOpen") {
        m_isClosetOpenFinished = true;
    } else if (request.api == "ClosetClose") {
        m_isClosetCloseFinished = true;
    }
}
