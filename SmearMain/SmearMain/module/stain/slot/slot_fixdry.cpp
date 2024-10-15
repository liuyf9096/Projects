#include "slot_fixdry.h"

SlotFixDry::SlotFixDry(int pos, const QString &group, QObject *parent)
    : SlotBase{pos, group, parent}
{

}

void SlotFixDry::handlePutinSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("fixdry_0", QTime::currentTime().toString("HH:mm:ss"));
}

void SlotFixDry::handleTakeoutSlide()
{
    Q_ASSERT(m_slide);
    m_slide->sql_recordStainStatus("fixdry_1", QTime::currentTime().toString("HH:mm:ss"));
}
