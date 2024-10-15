#include "m_export.h"
#include "rack/rt_rack_manager.h"
#include "rack/rt_rack.h"

MExport::MExport(const QString &mid, QObject *parent)
    : DModuleBase(mid, "Export", parent)
{
    m_state = RecycleState::Idle;

    m_exportTimer = new QTimer(this);
    m_exportTimer->setInterval(100);
    connect(m_exportTimer, &QTimer::timeout, this, &MExport::onExportTimer_slot);
}

void MExport::start()
{
    m_exportTimer->start();
    DModuleBase::start();
}

void MExport::reset()
{
    m_state = RecycleState::Idle;
    DModuleBase::reset();
}

void MExport::stop()
{
    m_exportTimer->stop();
    DModuleBase::stop();
}

bool MExport::recycleRack(QSharedPointer<RtRack> rack)
{
    if (rack != nullptr && m_state == RecycleState::Idle) {
        m_recycleRack = rack;
        m_isRecycleFinished = false;
        return true;
    }
    return false;
}

bool MExport::isFull()
{
    QString sensor = QString("%1_full").arg(mUserId);
    return dev->track()->checkSensorValue(sensor);
}

void MExport::onExportTimer_slot()
{
    switch (m_state) {
    case RecycleState::Idle:
        if (m_recycleRack != nullptr) {
            logProcess("Recycle", 0, "Idle", QString("Check Full"));
            dev->track()->cmd_CheckSensorValue();
            m_state = RecycleState::Check_Full_Status;
        }
        break;

    case RecycleState::Check_Full_Status:
        if (dev->track()->isFuncDone("CheckSensorValue")) {
            if (isFull() == false) {
                logProcess("Recycle", 1, "Recycle Rack to Export", QString("rack:%1").arg(m_recycleRack->rackid()));
                m_state = RecycleState::Send_Unload_Rack_Cmd;
            } else {
                dev->track()->setCheckSensorEnable(mid(), true);
                logProcess("Recycle", 1, "Export is Full");

                Exception e("RackExitFull_101", E_Level::Alarm, 101);
                e.e_msg = "Rack Exit is Full, please remove tested racks.";
                ExceptionCenter::GetInstance()->sendExceptionMessage(e);

                m_state = RecycleState::WaitF_UnFull;
            }
        } else if (dev->track()->getFuncResult("CheckSensorValue") > Func_Done) {
            m_state = RecycleState::Idle;
        }
        break;
    case RecycleState::WaitF_UnFull:
        if (isFull() == false) {
            logProcess("Recycle", 1, "Export is Not Full");
            dev->track()->setCheckSensorEnable(mid(), false);

            ExceptionCenter::GetInstance()->removeException("RackExitFull_101", true);

            m_state = RecycleState::Send_Unload_Rack_Cmd;
        }
        break;

    case RecycleState::Send_Unload_Rack_Cmd:
        dev->track()->cmd_Export_Unload();
        m_state = RecycleState::WaitF_Unload_Rack_Cmd_Done;
        break;
    case RecycleState::WaitF_Unload_Rack_Cmd_Done:
        if (dev->track()->isFuncDone("Export_Unload")) {
            m_isRecycleFinished = true;
            RtRackManager::GetInstance()->removeRackOne(m_recycleRack->rackid());
            m_recycleRack = nullptr;

            logProcess("Recycle", 99, "Finish");
            m_state = RecycleState::Finish;
        }
        break;

    case RecycleState::Finish:
        m_state = RecycleState::Idle;
        break;
    }
}
