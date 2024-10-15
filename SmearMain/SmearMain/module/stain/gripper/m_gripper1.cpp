#include "m_gripper1.h"
#include "gripper_manager.h"
#include "stain/stain_manager.h"
#include "sql/f_sql_database_manager.h"

MGripper1::MGripper1(const QString &mid, const QString &userid, QObject *parent)
    : MGripperBase(mid, userid, parent)
{
    mLeftOffset = 9;
    mRightOffset = 9;

    mImportPos = 26;
    mResetPos = 0;
    mAvoidPos = 21;

    coord.setName("G1");
    coord.setOffset(mLeftOffset, mRightOffset);

    _getOffset();
    _init();

    connect(FMessageCenter::GetInstance(), &FMessageCenter::gripperStatusInfo_signal,
            this, &MGripper1::onGripperStatusInfo_slot);
}

void MGripper1::_init()
{
    s_process = Idle;
    m_errorid.clear();
    coord.setCoord(mResetPos);
}

void MGripper1::resetCoord()
{
    coord.setCoord(mResetPos);
}

void MGripper1::_getOffset()
{
    FSqlDatabase *db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (db) {
        QJsonObject whereObj;
        whereObj.insert("gid", "g1");
        QJsonArray arr = db->selectRecord("gripper", whereObj);
        if (!arr.isEmpty()) {
            QJsonObject obj = arr.first().toObject();
            int left_offset = obj.value("left_offset").toInt();
            int right_offset = obj.value("right_offset").toInt();
            if (left_offset > 0 && left_offset < 15) {
                mLeftOffset = left_offset;
            }
            if (right_offset > 0 && right_offset < 15) {
                mRightOffset = right_offset;
            }
            coord.setOffset(mLeftOffset, mRightOffset);
            qDebug() << "[G1] left_offset:" << mLeftOffset << "right_offset:" << mRightOffset;
        }
    }
}

void MGripper1::state_init()
{
    _init();
}

bool MGripper1::cmd_Gripper(int from, int to, QString &errorid)
{
    if (from <= 10) {
        if (dev->stain()->checkSensorValue("StainOnly1_exist") || true) {//todo
            if (to > mImportPos) {
                m_api = "G1_stainonly1_withReset";
                m_toPos = mResetPos;
                return dev->stain()->cmd_G1_stainonly1_withReset(from, to);
            } else {
                m_api = "G1_stainonly1";
                m_toPos = to;
                return dev->stain()->cmd_G1_stainonly1(from, to);
            }
        } else {
            errorid = "StainOnly1_NOT_exist";
            qWarning() << "failed, StainOnly1 NOT exist.";
            return false;
        }
    } else if (from <= 20) {
        if (dev->stain()->checkSensorValue("StainOnly2_exist") || true) { //todo
            if (to > mImportPos) {
                m_api = "G1_stainonly2_withReset";
                m_toPos = mResetPos;
                return dev->stain()->cmd_G1_stainonly2_withReset(from, to);
            } else {
                m_api = "G1_stainonly2";
                m_toPos = to;
                return dev->stain()->cmd_G1_stainonly2(from, to);
            }
        } else {
            errorid = "StainOnly2_NOT_exist";
            qWarning() << "failed, StainOnly2 NOT exist.";
            return false;
        }
    } else {
        if (from == mImportPos) {
            m_api = "G1_fromImport";
            m_toPos = mResetPos;
            return dev->stain()->cmd_G1_fromImport(to);
        } else if (to > mImportPos) {
            m_api = "G1_escape";
            m_toPos = mAvoidPos;
            return dev->stain()->cmd_G1_escape(from, to);
        } else {
            m_api = "G1";
            m_toPos = to;
            return dev->stain()->cmd_G1(from, to);
        }
    }
    return false;
}

void MGripper1::onTaskTimer_slot()
{
    if (dev->stain()->isResetOk() == false) {
        return;
    }

    switch (s_process) {
    case Idle:
        if (m_requestList.isEmpty() == false)
        {
            for (int i = 0; i < m_requestList.count(); ++i)
            {
                GripperRequest req = m_requestList.at(i);

                if (req.from_groupid.contains("stainonly1")) {
                    if (dev->stain()->checkSensorValue("StainOnly1_exist") == false) {
//                        break;//todo
                    }
                } else if (req.from_groupid.contains("stainonly2")) {
                    if (dev->stain()->checkSensorValue("StainOnly2_exist") == false) {
//                        break;//todo
                    }
                }

                int pos = SlotsManager::GetInstance()->applySlotPos(req.to_groupid);
                if (pos >= 0) {
                    req.to_pos = pos;
                    m_Req = req;

                    logProcess("[G1]", 0, "apply pos", QString::number(req.to_pos));
                    m_requestList.removeAt(i);
                    s_process = Send_Trasnfer_Cmd;
                    break;
                }
            }
        }
        break;

    case Send_Trasnfer_Cmd:
    {
        bool ok = GripperManager::GetInstance()->requestMoveAccess(this, m_Req.from_pos, m_Req.to_pos);
        if (ok) {
            logProcess("[G1]", 1, QString("Move: from:%1 to:%2").arg(m_Req.from_pos).arg(m_Req.to_pos));
            bool ok1 = cmd_Gripper(m_Req.from_pos, m_Req.to_pos, m_errorid);
            if (ok1) {
                m_gripperOpenCount = 0;
                SlotsManager::GetInstance()->prepareSlotPos(m_Req.to_pos, m_Req.sid);
                StainManager::GetInstance()->takingOutSample(m_Req.from_groupid, m_Req.sid, m_Req.from_pos);
                s_process = WaitF_Transfer_Cmd_Done;
            } else {
                s_process = Error;
            }
        }
        break;
    }

    case WaitF_Transfer_Cmd_Done:
        if (dev->stain()->getFuncResult(m_api) == Func_Done)
        {
            m_pos = m_toPos;
            coord.setCoord(m_toPos);

            StainManager::GetInstance()->takeOutSample(m_Req.from_groupid, m_Req.sid, m_Req.from_pos);
            SlotsManager::GetInstance()->slideStainStart(m_Req.to_groupid, m_Req.sid, m_Req.to_pos);
            s_process = Finish;
        } else if (dev->stain()->getFuncResult(m_api) == Func_Fail) {
            m_errorid = m_api;
            s_process = Error;
        }
        break;

    case Finish:
        logProcess("[G1]", 99, "Finish");
        s_process = Idle;
        break;

    case Error:
        if (m_errorid.isEmpty()) {
            s_process = Send_Trasnfer_Cmd;
        }
        break;
    }
}

void MGripper1::onGripperStatusInfo_slot(const QJsonObject &obj)
{
    if (obj.contains("gripper_id")) {
#if 0
        int gripper_id = obj.value("gripper_id").toInt();
        int grip_state = obj.value("grip_state").toInt();
        if (gripper_id == 2) {
            if (s_process == WaitF_Transfer_Cmd_Done) {
                if (grip_state == 0) {
                    /* Close */
                    StainManager::GetInstance()->takeOutSample(m_Req.from_groupid, m_Req.sid, m_Req.from_pos);
                } else if (grip_state == 1) {
                    /* Open */
                    m_gripperOpenCount++;
                    if (m_gripperOpenCount == 2) {
                        SlotsManager::GetInstance()->slideStainStart(m_Req.to_groupid, m_Req.sid, m_Req.to_pos);
                    }
                }
            }
        }
#endif
    }
}
