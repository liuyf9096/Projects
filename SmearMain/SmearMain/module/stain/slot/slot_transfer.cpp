#include "slot_transfer.h"

SlotTransfer::SlotTransfer(int pos, const QString &group, QObject *parent)
    : SlotBase{pos, group, parent}
{
    m_durationTimer->setInterval(10);
}

void SlotTransfer::handlePutinSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("transfer_0", QTime::currentTime().toString("HH:mm:ss"));
}

void SlotTransfer::handleTakeoutSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("transfer_1", QTime::currentTime().toString("HH:mm:ss"));
}
