#include "m_stain_aging_process.h"
#include "messagecenter/f_message_center.h"

MStainAgingProcess::MStainAgingProcess(QObject *parent)
    : MProcessBase{"stainAgingProcess", "stainAging", parent}
{
    _init();

    QJsonObject whereObj;
    whereObj.insert("project", "stain");
    auto arr = mRecordDb->selectRecord("aging", whereObj);
    if (arr.count() > 0) {
        QJsonObject obj = arr.first().toObject();
        m_doneCount = obj.value("count").toInt();
    }
}

void MStainAgingProcess::_init()
{
    pos = 0;
    r_pos = 0;
    m_state = Idle;
}

void MStainAgingProcess::state_init()
{
    _init();
}

bool MStainAgingProcess::startProcess(quint64 id)
{
    m_reqId = id;

    if (mTimer->isActive() == false) {
        mTimer->start();
    }
    sendOkMessage(id);
    qInfo() << "Start Process:" << mUserId << "id:" << m_reqId;
    return true;
}

void MStainAgingProcess::resetProcess(quint64 id)
{
    state_init();
    dev->stain()->cmd_G1_Reset();
    dev->stain()->cmd_G2_Reset();
    dev->sample()->cmd_StainCart_Reset();

    sendOkMessage(id);
}

/* |wash  |c2b   |c2a   |tr |c1    |import |a1    |fix   |stainOnly2 |stainOnly1 */
/* |62-61 |60-51 |50-41 |40 |39-33 |32     |31-25 |24-21 |20-11      |10-1       */

/* |recycle */
/* |66-75 */

void MStainAgingProcess::onTimer_slot()
{
    process2();

#if 0
    switch (m_state) {
    case Idle:
    {
        bool ok = dev->stain()->cmd_NewRecycleBox();
        if (ok) {

            JPacket p(PacketType::Notification);
            p.module = "StainAging";
            p.api = "SuccessCount";

            QJsonObject obj;
            obj.insert("count", m_doneCount);
            p.paramsValue = obj;

            sendUIMessage(p);

            m_state = From_Import_To_Fix_1;
        }
    }
        break;

    case Prepare_1:
    {
        bool ok = dev->sample()->cmd_StainCart_Reset();
        if (ok) {
            m_state = Prepare_2;
        }
    }
        break;

    case Prepare_2:
        if (dev->sample()->isFuncDone("StainCart_Reset")) {
            m_state = From_Import_To_Fix_1;
        }
        break;

    case From_Import_To_Fix_1:
        dev->sample()->cmd_StainCart_ToStainImportWithoutHeat();
        m_state = From_Import_To_Fix_2;
        break;
    case From_Import_To_Fix_2:
        if (dev->sample()->isFuncDone("StainCart_ToStainImportWithoutHeat")) {
            m_state = From_Import_To_Fix_3;
        }
        break;

    case From_Import_To_Fix_3:
        dev->stain()->cmd_G1(32, 21);
        m_state = From_Import_To_Fix_4;
        break;
    case From_Import_To_Fix_4:
        if (dev->stain()->isFuncDone("G1")) {
            pos = 21;
            m_state = From_Fix_To_A1_1;
        }
        break;

    case From_Fix_To_A1_1:
        if (pos < 31) {
            dev->stain()->cmd_G1(pos, pos + 1);
            pos++;
            m_state = From_Fix_To_A1_2;
        } else {
            m_state = From_Fix_To_A1_3;
        }
        break;
    case From_Fix_To_A1_2:
        if (dev->stain()->isFuncDone("G1")) {
            m_state = From_Fix_To_A1_1;
        }
        break;

    case From_Fix_To_A1_3:
        dev->stain()->cmd_G1(31, 33);
        m_state = From_Fix_To_A1_4;
        break;

    case From_Fix_To_A1_4:
        if (dev->stain()->isFuncDone("G1")) {
            pos = 33;
            m_state = From_A1_To_Tr_1;
        }
        break;

    case From_A1_To_Tr_1:
        if (pos < 39) {
            dev->stain()->cmd_G1(pos, pos + 1);
            pos++;
            m_state = From_A1_To_Tr_2;
        } else {
            m_state = From_A1_To_Tr_3;
        }
        break;
    case From_A1_To_Tr_2:
        if (dev->stain()->isFuncDone("G1")) {
            m_state = From_A1_To_Tr_1;
        }
        break;

    case From_A1_To_Tr_3:
        dev->stain()->cmd_G1_withReset(39, 40);
        m_state = From_A1_To_Tr_4;
        break;
    case From_A1_To_Tr_4:
        if (dev->stain()->isFuncDone("G1_withReset")) {
            pos = 40;
            m_state = From_Tr_To_Wash_1;
        }
        break;

    case From_Tr_To_Wash_1:
        if (pos < 62) {
            dev->stain()->cmd_G2(pos, pos + 1);
            pos++;
            m_state = From_Tr_To_Wash_2;
        } else {
            pos = 62;
            r_pos = 66;
            m_state = From_Wash_To_Recycle_1;
        }
        break;
    case From_Tr_To_Wash_2:
        if (dev->stain()->isFuncDone("G2")) {
            m_state = From_Tr_To_Wash_1;
        }
        break;

    case From_Wash_To_Recycle_1:
        dev->stain()->cmd_G2_recycle(pos, r_pos);
        pos = (pos == 62) ? 61 : 62;
        m_state = From_Wash_To_Recycle_2;
        break;
    case From_Wash_To_Recycle_2:
        if (dev->stain()->isFuncDone("G2_recycle")) {
            m_state = From_Wash_To_Recycle_3;
        }
        break;

    case From_Wash_To_Recycle_3:
        if (r_pos < 75) {
            dev->stain()->cmd_G2_recycle_reverse(r_pos, pos);
            r_pos++;
            m_state = From_Wash_To_Recycle_4;
        } else {
            m_state = Renew_Slide_1;
        }
        break;
    case From_Wash_To_Recycle_4:
        if (dev->stain()->isFuncDone("G2_recycle_reverse")) {
            m_state = From_Wash_To_Recycle_1;
        }
        break;

    case Renew_Slide_1:
        dev->stain()->cmd_G2_recycle_reverse(75, 40);
        m_state = Renew_Slide_2;
        break;
    case Renew_Slide_2:
        if (dev->stain()->isFuncDone("G2_recycle_reverse")) {
            dev->stain()->cmd_G2_Reset();
            m_state = Renew_Slide_3;
        }
        break;

    case Renew_Slide_3:
        if (dev->stain()->isFuncDone("G2_Reset")) {
            dev->stain()->cmd_G1_withReset(40, 32);
            m_state = Renew_Slide_4;
        }
        break;

    case Renew_Slide_4:
        if (dev->stain()->isFuncDone("G1_withReset")) {
            dev->sample()->cmd_StainCart_Reset();
            m_state = Renew_Slide_5;
        }
        break;

    case Renew_Slide_5:
        if (dev->sample()->isFuncDone("StainCart_Reset")) {
            m_doneCount++;

            JPacket p(PacketType::Notification);
            p.module = "StainAging";
            p.api = "SuccessCount";

            QJsonObject obj;
            obj.insert("count", m_doneCount);
            p.paramsValue = obj;

            sendUIMessage(p);

            /* record */
            auto record = RtMachineRecord::GetInstance();
            record->agingStain = m_doneCount;
            record->write();

            m_state = Idle;
        }
        break;

    default:
        break;
    }
#endif
}

void MStainAgingProcess::process2()
{
    switch (m_state) {
    case Idle:
    {
        bool ok = dev->stain()->cmd_ComboLoop();
        if (ok) {
            JPacket p(PacketType::Notification);
            p.module = "StainAging";
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
        if (dev->stain()->isFuncDone("ComboLoop")) {
            m_state = Finish;
        }
        break;

    case Finish:
    {
        m_doneCount++;

        JPacket p(PacketType::Notification);
        p.module = "StainAging";
        p.api = "SuccessCount";

        QJsonObject obj;
        obj.insert("count", m_doneCount);
        p.paramsValue = obj;

        sendUIMessage(p);

        /* record */
        mRecordDb->updateRecord("aging", {{"count", m_doneCount}}, {{"project", "stain"}});

        m_state = Idle;
    }
        break;
    default:
        break;
    }
}

