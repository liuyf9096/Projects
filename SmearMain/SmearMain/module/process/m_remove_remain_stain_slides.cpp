#include "m_remove_remain_stain_slides.h"
#include "sql/f_sql_database_manager.h"
#include "stain/slot/slots_manager.h"
#include "stain/gripper/gripper_manager.h"
#include "stain/recyclebox/m_recyclebox_mgr.h"

MRemoveRemainStainSlides::MRemoveRemainStainSlides(QObject *parent)
    : MProcessBase{"removeremainstainslides", "removeremainstainslides", parent}
{
    state_init();
}

void MRemoveRemainStainSlides::state_init()
{
    s_rm = RemoveState::Idle;
}

void MRemoveRemainStainSlides::onTimer_slot()
{
    switch (s_rm) {
    case RemoveState::Idle:
        s_rm = RemoveState::Get_New_RecycleBox;
        break;

    case RemoveState::Get_New_RecycleBox:
        MRecycleBoxMgr::GetInstance()->startRun(true);
        MRecycleBoxMgr::GetInstance()->setClearMode(true);
        s_rm = RemoveState::WaitF_New_RecycleBox_Done;
        break;
    case RemoveState::WaitF_New_RecycleBox_Done:
    {
        auto state = MRecycleBoxMgr::GetInstance()->getBoxState();
        if (state == MRecycleBoxMgr::None) {
            s_rm = RemoveState::Error;
        } else if (state == MRecycleBoxMgr::Available) {
            s_rm = RemoveState::Assign_Remain_Slides;
        }
    }
        break;

    case RemoveState::Assign_Remain_Slides:
        m_remainSlidesArr = handleRemainSlides();
        if (m_remainSlidesArr.isEmpty() == false) {
            s_rm = RemoveState::Remove;
        } else {
            s_rm = RemoveState::Finish;
        }
        break;

    case RemoveState::Remove:
        SlotsManager::GetInstance()->handleRemainSlides(m_remainSlidesArr);
        s_rm = RemoveState::WaitF_Remove_Done;
        break;
    case RemoveState::WaitF_Remove_Done:
    {
        QJsonArray arr = mRecordDb->selectRecord("slot", "slide_id!='' AND group_id!='import' AND group_id!='StainOnly' AND group_id!='recycle'");
        if (arr.isEmpty()) {
            qDebug() << "All Remain Slides Removed OK.";
            s_rm = RemoveState::Remove_RecycleBox;
        }
    }
        break;

    case RemoveState::Remove_RecycleBox:
    {
        bool ok = MRecycleBoxMgr::GetInstance()->ejectCurrentBox();
        if (ok) {
            s_rm = RemoveState::Finish;
        }
    }
        break;
    case RemoveState::WaitF_Remove_RecycleBox_Done:
        if (MRecycleBoxMgr::GetInstance()->isIdle()) {
            s_rm = RemoveState::Finish;
        }
        break;

    case RemoveState::Finish:
        m_isFinished = true;
        MRecycleBoxMgr::GetInstance()->setClearMode(false);
        ExceptionCenter::GetInstance()->removeException("StainSlideRemain_101", false);
        sendOkMessage();
        mTimer->stop();
        break;
    case RemoveState::Error:
        m_isFinished = true;
        MRecycleBoxMgr::GetInstance()->setClearMode(false);
        sendErrorMessage();
        mTimer->stop();
        break;
    }
}

QJsonArray MRemoveRemainStainSlides::handleRemainSlides()
{
    QJsonArray res_arr;
    QJsonArray arr = mRecordDb->selectRecord("slot", "slide_id!='' AND group_id!='import' AND group_id!='StainOnly' AND group_id!='recycle'");
    qDebug() << "Remain Slides:" << arr.count() << arr;

    bool logclear = FCommon::GetInstance()->getConfigValue("log", "clear_sample_history").toBool();

    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString slide_id = obj.value("slide_id").toString();
        QString slide_uid = obj.value("slide_uid").toString();
        QString sample_uid = obj.value("sample_uid").toString();
        QString sample_id = obj.value("sample_id").toString();
        QString group_id = obj.value("group_id").toString();

        QSharedPointer<RtSlide> slide;
        if (logclear == true) {
            slide = RtSampleManager::GetInstance()->NewSampleSlide();
        } else {
            slide = RtSampleManager::GetInstance()->NewSampleSlide(slide_id);
        }

        slide->setSlideUID(slide_uid);
        slide->setSampleUID(sample_uid);
        slide->setSampleID(sample_id);
        slide->setRemained();

        if (group_id == "fix" || group_id == "fixdry") {
            slide->addStainProcess("c1");
        }
        if (group_id != "wash") {
            slide->addStainProcess("wash");
        }

        slide->setPrintEnable(false);
        slide->setSmearEnable(false);
        slide->setStainEnable(false);

        obj.insert("slide_id", slide->slide_id());
        res_arr.append(obj);
    }

    return res_arr;
}

