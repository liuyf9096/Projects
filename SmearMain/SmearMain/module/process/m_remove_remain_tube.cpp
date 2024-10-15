#include "m_remove_remain_tube.h"
#include "record/rt_machine_record.h"
#include "track/m_emergency.h"
#include "exception/exception_center.h"

MRemoveRemainTube::MRemoveRemainTube(QObject *parent)
    : MProcessBase{"removeremiantube", "removeremiantube", parent}
{
    _init();
}

void MRemoveRemainTube::_init()
{
    m_pos = Unknown;
    m_waitTime = 0;
    s_removeCart = RemoveState::Idle;
    s_removeMix = RemoveState::Idle;
    s_removeGrip = RemoveState::Idle;
}

void MRemoveRemainTube::startProcess(TubePosition pos, quint64 id)
{
    if (MProcessBase::startProcess(id) == true) {
        m_pos = pos;
    }
}

void MRemoveRemainTube::state_init()
{
    _init();
}

void MRemoveRemainTube::onTimer_slot()
{
    if (m_pos == TubePosition::InCart) {
        removeCartTube();
    } else if (m_pos == TubePosition::InMix) {
        removeMixTube();
    } else if (m_pos == TubePosition::InGrip) {
        removeGripTube();
    }
}

void MRemoveRemainTube::removeCartTube()
{
    switch (s_removeCart) {
    case RemoveState::Idle:
        s_removeCart = RemoveState::Reset_Closet;
        break;

    case RemoveState::Reset_Closet:
        mEmerg->Closet_Close();
        s_removeCart = RemoveState::WaitF_Reset_Closet_Done;
        break;
    case RemoveState::WaitF_Reset_Closet_Done:
        if (mEmerg->isClosetCloseDone()) {
            s_removeCart = RemoveState::Move_To_Cap_Detect;
        }
        break;

    case RemoveState::Move_To_Cap_Detect:
        dev->sample()->cmd_CartToCapDetect();
        s_removeCart = RemoveState::WaitF_Move_To_Detect_Done;
        break;
    case RemoveState::WaitF_Move_To_Detect_Done:
        if (dev->sample()->isFuncDone("CartToCapDetect")) {
            s_removeCart = RemoveState::Check_Cap_Status;
        }
        break;

    case RemoveState::Check_Cap_Status:
        dev->sample()->cmd_CheckSensorValue();
        s_removeCart = RemoveState::WaitF_Check_Res;
        break;
    case RemoveState::WaitF_Check_Res:
        if (dev->sample()->isFuncDone("CheckSensorValue")) {
            if (dev->sample()->checkSensorValue("TubeCap") == false) {
                s_removeCart = RemoveState::Remove_Last_Tube;
            } else {
                s_removeCart = RemoveState::Cap_Tube;
            }
        }
        break;

    case RemoveState::Cap_Tube:
        dev->sample()->cmd_CartToExitPos_Capping2();
        s_removeCart = RemoveState::WaitF_Cap_Tube_Done;
        break;
    case RemoveState::WaitF_Cap_Tube_Done:
        if (dev->sample()->isFuncDone("CartToExitPos_Capping2")) {
            s_removeCart = RemoveState::Remove_Last_Tube;
        }
        break;

    case RemoveState::Remove_Last_Tube:
        dev->sample()->cmd_ReturnTubeFrCart(1); /* pos: (0:rack, 1:closet) */
        s_removeCart = RemoveState::WaitF_Remove_Last_Tube_Done;
        break;
    case RemoveState::WaitF_Remove_Last_Tube_Done:
        if (dev->sample()->isFuncDone("ReturnTubeFrCart")) {
            /* record */
            QJsonObject setObj;
            setObj.insert("sid", "");
            setObj.insert("sample_uid", "");
            setObj.insert("isCapped", 1);
            mRecordDb->updateRecord("doing_sample", setObj, {{"module", "cart"}});
            s_removeCart = RemoveState::Open_Closet;
        }
        break;

    case RemoveState::Open_Closet:
        mEmerg->Closet_Open();
        s_removeCart = RemoveState::WaitF_Open_Closet_Done;
        break;
    case RemoveState::WaitF_Open_Closet_Done:
        if (mEmerg->isClosetOpenDone()) {
            m_waitTime = 0;
            s_removeCart = RemoveState::WaitF_Sometime_Done;
        }
        break;

    case RemoveState::WaitF_Sometime_Done:
        m_waitTime++;
        if (m_waitTime > 50) {
            s_removeCart = RemoveState::Close_Closet;
        }
        break;

    case RemoveState::Close_Closet:
        mEmerg->Closet_Close();
        s_removeCart = RemoveState::WaitF_Close_Closet_Done;
        break;
    case RemoveState::WaitF_Close_Closet_Done:
        if (mEmerg->isClosetCloseDone()) {
            s_removeCart = RemoveState::Finish;
        }
        break;

    case RemoveState::Finish:
        ExceptionCenter::GetInstance()->removeException("TubeRemain_101", false);

        sendOkMessage();
        mTimer->stop();
        m_isFinished = true;
        s_removeCart = RemoveState::Idle;
        break;

    default:
        break;
    }
}

void MRemoveRemainTube::removeMixTube()
{
    switch (s_removeMix) {
    case RemoveState::Idle:
        s_removeMix = RemoveState::Reset_Closet;
        break;

    case RemoveState::Reset_Closet:
        mEmerg->Closet_Close();
        s_removeMix = RemoveState::WaitF_Reset_Closet_Done;
        break;
    case RemoveState::WaitF_Reset_Closet_Done:
        if (mEmerg->isClosetCloseDone()) {
            s_removeMix = RemoveState::Remove_Last_Tube;
        }
        break;

    case RemoveState::Remove_Last_Tube:
        dev->sample()->cmd_ReturnTubeFrMix(1); /* pos: (0:rack, 1:closet) */
        s_removeMix = RemoveState::WaitF_Remove_Last_Tube_Done;
        break;
    case RemoveState::WaitF_Remove_Last_Tube_Done:
        if (dev->sample()->isFuncDone("ReturnTubeFrMix")) {
            /* record */
            QJsonObject setObj;
            setObj.insert("sid", "");
            setObj.insert("sample_uid", "");
            mRecordDb->updateRecord("doing_sample", setObj, {{"module", "mix"}});
            s_removeMix = RemoveState::Open_Closet;
        }
        break;

    case RemoveState::Open_Closet:
        mEmerg->Closet_Open();
        s_removeMix = RemoveState::WaitF_Open_Closet_Done;
        break;
    case RemoveState::WaitF_Open_Closet_Done:
        if (mEmerg->isClosetOpenDone()) {
            m_waitTime = 0;
            s_removeMix = RemoveState::WaitF_Sometime_Done;
        }
        break;

    case RemoveState::WaitF_Sometime_Done:
        m_waitTime++;
        if (m_waitTime > 50) {
            s_removeMix = RemoveState::Close_Closet;
        }
        break;

    case RemoveState::Close_Closet:
        mEmerg->Closet_Close();
        s_removeMix = RemoveState::WaitF_Close_Closet_Done;
        break;
    case RemoveState::WaitF_Close_Closet_Done:
        if (mEmerg->isClosetCloseDone()) {
            s_removeMix = RemoveState::Finish;
        }
        break;

    case RemoveState::Finish:
        ExceptionCenter::GetInstance()->removeException("TubeRemain_101", false);

        sendOkMessage();
        mTimer->stop();
        m_isFinished = true;
        s_removeMix = RemoveState::Idle;
        break;

    default:
        break;
    }
}

void MRemoveRemainTube::removeGripTube()
{
    switch (s_removeGrip) {
    case RemoveState::Idle:
        s_removeGrip = RemoveState::Reset_Closet;
        break;

    case RemoveState::Finish:
        sendOkMessage();
        mTimer->stop();
        m_isFinished = true;
        s_removeGrip = RemoveState::Idle;
        break;

    default:
        break;
    }
}
