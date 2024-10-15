#include "m_smear_aging_process.h"
#include "messagecenter/f_message_center.h"

MSmearAgingProcess::MSmearAgingProcess(QObject *parent)
    : MProcessBase{"smearAgingProcess", "smearAging", parent}
{
    _init();

    auto arr = mRecordDb->selectRecord("aging", QJsonObject({{"project", "smear"}}));
    if (arr.count() > 0) {
        QJsonObject obj = arr.first().toObject();
        m_doneCount = obj.value("count").toInt();
    }
}

void MSmearAgingProcess::_init()
{
    m_state = Idle;
}

void MSmearAgingProcess::state_init()
{
    _init();
}

bool MSmearAgingProcess::startProcess(quint64 id)
{
    m_reqId = id;

    if (mTimer->isActive() == false) {
        mTimer->start();
    }
    qInfo() << "Start Process:" << mUserId << "id:" << m_reqId;
    sendOkMessage(id);
    return true;
}

void MSmearAgingProcess::resetProcess(quint64 id)
{
    state_init();
    dev->smear()->reset();
    dev->sample()->reset();
    sendOkMessage(id);
}

void MSmearAgingProcess::onTimer_slot()
{
#if 0
    process1();
#else
    process2();
#endif
}

void MSmearAgingProcess::process1()
{
    switch (m_state) {
    case Idle:
    {
        bool ok = dev->smear()->cmd_NewSlide1_ToAddSamplingPos();
        if (ok) {
            JPacket p(PacketType::Notification);
            p.module = "SmearAging";
            p.api = "SuccessCount";

            QJsonObject obj;
            obj.insert("count", m_doneCount);
            p.paramsValue = obj;

            sendUIMessage(p);
        }

        m_state = Get_New_Tube_1;
    }
        break;

    case Get_New_Tube_1:
        dev->sample()->cmd_GetNewTube(1);
        m_state = Get_New_Tube_2;
        break;
    case Get_New_Tube_2:
        if (dev->sample()->isFuncDone("GetNewTube")) {
            m_state = Mix_Tube_1;
        }
        break;

    case Mix_Tube_1:
        //todo
        m_state = Mix_Tube_2;
        break;
    case Mix_Tube_2:
        if (dev->sample()->isFuncDone("TubeToNormal")) {
            m_state = Sampling_1;
        }
        break;

    case Sampling_1:
//        dev->sample()->cmd_NeedleStab_TakeSample_Aging(200);
        m_state = Sampling_2;
        break;
    case Sampling_2:
        if (dev->sample()->isFuncDone("NeedleStab_TakeSample_Aging")) {
            m_state = Sampling_3;
        }
        break;
    case Sampling_3:
//        dev->sample()->cmd_PrepareSampling_Aging();
        m_state = Sampling_4;
        break;
    case Sampling_4:
        if (dev->sample()->isFuncDone("PrepareSampling_Aging")) {
            m_state = Sampling_5;
        }
        break;
    case Sampling_5:
//        dev->sample()->cmd_AddSample_Aging(200);
        m_state = Sampling_6;
        break;
    case Sampling_6:
        if (dev->sample()->isFuncDone("AddSample_Aging")) {
            m_state = Smear_1;
        }
        break;

    case Smear_1:
        dev->smear()->cmd_Smear(3600,1200,800,2000,9000,2100,2000,2000,6000,2000);
        m_state = Smear_2;
        break;
    case Smear_2:
        if (dev->smear()->isFuncDone("Smear")) {
            m_state = Smear_3;
        }
        break;

    case Smear_3:
        dev->smear()->cmd_CleanSmearBlade_Aging();
        m_state = Recycle_Tube_1;
        break;
    case Smear_4:
        if (dev->smear()->isFuncDone("CleanSmearBlade_Aging")) {
            m_state = Recycle_Tube_1;
        }
        break;

    case Smear_5:
        dev->smear()->cmd_SmearCart_Reset();
        m_state = Smear_6;
        break;
    case Smear_6:
        if (dev->smear()->isFuncDone("SmearCart_Reset")) {
            m_state = Recycle_Tube_1;
        }
        break;

    case Recycle_Tube_1:
        dev->sample()->cmd_ReturnTubeFrCart(1);
        m_state = Recycle_Tube_2;
        break;
    case Recycle_Tube_2:
        if (dev->sample()->isFuncDone("ReturnTubeFrCart")) {
            m_state = Finish;
        }
        break;

    case Finish:
    {
        m_doneCount++;

        JPacket p(PacketType::Notification);
        p.module = "SmearAging";
        p.api = "SuccessCount";

        QJsonObject obj;
        obj.insert("count", m_doneCount);
        p.paramsValue = obj;

        sendUIMessage(p);

        /* record */
        mRecordDb->updateRecord("aging", {{"count", m_doneCount}}, {{"project", "smear"}});

        m_state = Idle;
    }
        break;
    default:
        break;
    }
}

void MSmearAgingProcess::process2()
{
    switch (m_state) {
    case Idle:
    {
        bool ok = dev->sample()->cmd_ComboLoop();
        dev->smear()->cmd_ComboLoop();
        if (ok) {
            JPacket p(PacketType::Notification);
            p.module = "SmearAging";
            p.api = "SuccessCount";

            QJsonObject obj;
            obj.insert("count", m_doneCount);
            p.paramsValue = obj;

            sendUIMessage(p);
        }

        m_state = State1;
    }
        break;

    case State1:
        if (dev->sample()->isFuncDone("ComboLoop")
                && dev->smear()->isFuncDone("ComboLoop")) {
            m_state = Finish;
        }
        break;

    case Finish:
    {
        m_doneCount++;

        JPacket p(PacketType::Notification);
        p.module = "SmearAging";
        p.api = "SuccessCount";

        QJsonObject obj;
        obj.insert("count", m_doneCount);
        p.paramsValue = obj;

        sendUIMessage(p);

        /* record */
        mRecordDb->updateRecord("aging", {{"count", m_doneCount}}, {{"project", "smear"}});

        m_state = Idle;
    }
        break;
    default:
        break;
    }
}
