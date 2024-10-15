#include "stainonly2.h"
#include "light/m_lights.h"

StainOnly2::StainOnly2(const QString &mid, int from, int to, QObject *parent)
    : StainOnlyBase{mid, "stainonly2", from, to, parent}
{

}

void StainOnly2::setLightRun()
{
//    MLights::GetInstance()->setStainOnly2RunState(RunState::Run);
}

void StainOnly2::setLightStop()
{
//    MLights::GetInstance()->setStainOnly2RunState(RunState::Stop);
}

void StainOnly2::setLightError()
{
//    MLights::GetInstance()->setStainOnly2AlarmStatus(AlarmStatus::Error);
}

void StainOnly2::setLightAlarm()
{
//    MLights::GetInstance()->setStainOnly2AlarmStatus(AlarmStatus::Alarm);
}

void StainOnly2::clearLightAlarm()
{
//    MLights::GetInstance()->setStainOnly2AlarmStatus(AlarmStatus::Good);
}
