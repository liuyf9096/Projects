#include "m_export.h"

#include "rack/rt_rack.h"

MExport::MExport(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    m_isEnable = false;
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
    return dev->track1()->checkSensorValue(sensor);
}

void MExport::onExportTimer_slot()
{
    switch (m_state) {
    case RecycleState::Idle:
        if (m_recycleRack != nullptr) {
            logProcess("Recycle", 0, "Idle", QString("Check Full"));
            dev->track1()->cmd_CheckSensorValue();
            m_state = RecycleState::Check_Full_Status;
        }
        break;

    case RecycleState::Check_Full_Status:
        if (dev->track1()->isFuncDone("CheckSensorValue") == true) {
            if (isFull() == false) {
                logProcess("Recycle", 1, "Recycle Rack to Export", QString("rack:%1").arg(m_recycleRack->rackid()));
                m_state = RecycleState::Send_Unload_Rack_Cmd;
            } else {
                dev->track1()->setCheckSensorEnable(mid(), true);
                logProcess("Recycle", 1, "Export is Full");
                m_state = RecycleState::WaitF_UnFull;
            }
        }
        break;
    case RecycleState::WaitF_UnFull:
        if (isFull() == false) {
            logProcess("Recycle", 1, "Export is Not Full");
            dev->track1()->setCheckSensorEnable(mid(), false);
            m_state = RecycleState::Send_Unload_Rack_Cmd;
        }
        break;

    case RecycleState::Send_Unload_Rack_Cmd:
        cmd_UnloadRack();
        m_state = RecycleState::WaitF_Unload_Rack_Cmd_Done;
        break;
    case RecycleState::WaitF_Unload_Rack_Cmd_Done:
        if (isUnloadRack_Done() == true) {
            logProcess("Recycle", 2, "Finish");

            m_isRecycleFinished = true;
            m_recycleRack = nullptr;
            m_state = RecycleState::Finish;
        }
        break;

    case RecycleState::Finish:
        m_state = RecycleState::Idle;
        break;
    }
}
