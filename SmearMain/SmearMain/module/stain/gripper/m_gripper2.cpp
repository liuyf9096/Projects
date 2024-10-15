#include "m_gripper2.h"
#include "gripper_manager.h"
#include "stain/stain_manager.h"
#include "stain/slot/slots_manager.h"
#include "stain/recyclebox/m_recyclebox_mgr.h"
#include "sql/f_sql_database_manager.h"

MGripper2::MGripper2(const QString &mid, const QString &userid, QObject *parent)
    : MGripperBase(mid, userid, parent)
    , mRecycleBox(nullptr)
{
    mLeftOffset = 9;
    mRightOffset = 9;

    mAvoidPos = 50;
    mResetPos = 52;
    mRecyclePos = 53;

    coord.setName("G2");
    coord.setOffset(mLeftOffset, mRightOffset);

    _getOffset();
    _init();
}

void MGripper2::_init()
{
    s_process = Idle;
    m_errorid.clear();
    coord.setCoord(mResetPos);
}

void MGripper2::resetCoord()
{
    coord.setCoord(mResetPos);
}

void MGripper2::_getOffset()
{
    FSqlDatabase *db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (db) {
        QJsonObject whereObj;
        whereObj.insert("gid", "g2");
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
            qDebug() << "[G2] left_offset:" << mLeftOffset << "right_offset:" << mRightOffset;
        }
    }
}

void MGripper2::state_init()
{
    _init();
}

bool MGripper2::cmd_Gripper(int from_pos, int to_pos, QString &errorid)
{
    if (to_pos < mAvoidPos) {
        m_api = "G2_escape";
        m_toPos = mAvoidPos;
        return dev->stain()->cmd_G2_escape(from_pos, to_pos);
    } else if (to_pos >= mRecyclePos) {
        if (mRecycleBox && mRecycleBox->isFull() == false) {
            m_api = "G2_recycle";
            m_toPos = mAvoidPos;
            return dev->stain()->cmd_G2_recycle(from_pos, to_pos);
        } else {
            errorid = "Recycle box error.";
            return false;
        }
    } else {
        m_api = "G2";
        m_toPos = to_pos;
        return dev->stain()->cmd_G2(from_pos, to_pos);
    }
    return false;
}

void MGripper2::onTaskTimer_slot()
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
                if (req.to_groupid.contains("recycle"))
                {
                    auto box = RecycleBoxMgr->getCurrentBox();
                    if (box)
                    {
                        req.to_pos = box->getVacantPos();
                        mRecycleBox = box;

                        m_Req = req;
                        m_requestList.removeAt(i);

                        logProcess("[G2]", 0, "recycle slide box:", box->bid());
                        s_process = Send_Trasnfer_Cmd;
                        break;
                    }
                }
                else
                {
                    int pos = SlotsManager::GetInstance()->applySlotPos(req.to_groupid);
                    if (pos >= 0)
                    {
                        req.to_pos = pos;
                        m_Req = req;

                        logProcess("[G2]", 0, "apply pos", QString::number(req.to_pos));
                        m_requestList.removeAt(i);
                        s_process = Send_Trasnfer_Cmd;
                        break;
                    }
                }
            }
        }
        break;

    case Send_Trasnfer_Cmd:
    {
        bool ok = GripperManager::GetInstance()->requestMoveAccess(this, m_Req.from_pos, m_Req.to_pos);
        if (ok) {
            logProcess("[G2]", 1, QString("Move: from:%1 to:%2").arg(m_Req.from_pos).arg(m_Req.to_pos));
            bool ok_1 = cmd_Gripper(m_Req.from_pos, m_Req.to_pos, m_errorid);
            if (ok_1) {
                m_gripperOpenCount = 0;
                SlotsManager::GetInstance()->prepareSlotPos(m_Req.to_pos, m_Req.sid);
                s_process = WaitF_Transfer_Cmd_Done;
            }
        }
        break;
    }

    case WaitF_Transfer_Cmd_Done:
        if (dev->stain()->getFuncResult(m_api) == Func_Done)
        {
            m_pos = m_toPos;
            coord.setCoord(m_toPos);

            if (m_Req.to_groupid.contains("recycle")) {
                if (mRecycleBox) {
                    mRecycleBox->append(m_Req.to_pos, m_Req.sid);
                } else {
                    qWarning() << "G2 No recycle box.";
                }
            } else {
                SlotsManager::GetInstance()->slideStainStart(m_Req.to_groupid, m_Req.sid, m_Req.to_pos);
            }
            SlotsManager::GetInstance()->takeOutSample(m_Req.from_groupid, m_Req.sid, m_Req.from_pos);
            s_process = Finish;
        } else if (dev->stain()->getFuncResult(m_api) == Func_Fail) {
            s_process = Error;
        }
        break;

    case Finish:
        mRecycleBox = nullptr;

        logProcess("[G2]", 99, "Finish");
        s_process = Idle;
        break;

    case Error:
        if (m_errorid.isEmpty()) {
            m_errorid.clear();
            s_process = Send_Trasnfer_Cmd;
        }
        break;
    }
}

void MGripper2::onGripperStatusInfo_slot(const QJsonObject &obj)
{
    if (obj.contains("gripper_id")) {
#if 0
        int gripper_id = obj.value("gripper_id").toInt();
        int grip_state = obj.value("grip_state").toInt();
        if (gripper_id == 3) {
            if (s_process == WaitF_Transfer_Cmd_Done) {
                if (grip_state == 0) {
                    /* Close */
                    StainManager::GetInstance()->takeOutSample(m_Req.from_groupid, m_Req.sid, m_Req.from_pos);
                } else if (grip_state == 1) {
                    /* Open */
                    m_gripperOpenCount++;
                    if (m_gripperOpenCount == 2) {
                        if (m_Req.to_groupid.contains("recycle")) {
                            if (mRecycleBox) {
                                mRecycleBox->append(m_Req.to_pos, m_Req.sid);
                            } else {
                                qWarning() << "G2 No recycle box.";
                            }
                        } else {
                            SlotsManager::GetInstance()->slideStainStart(m_Req.to_groupid, m_Req.sid, m_Req.to_pos);
                        }
                    }
                }
            }
        }
#endif
    }
}

