#include "stainonly1.h"
#include "light/m_lights.h"

StainOnly1::StainOnly1(const QString &mid, int from, int to, QObject *parent)
    : StainOnlyBase{mid, "stainonly1", from, to, parent}
{

}

void StainOnly1::setLightRun()
{
//    MLights::GetInstance()->setStainOnly1RunState(RunState::Run);
}

void StainOnly1::setLightStop()
{
//    MLights::GetInstance()->setStainOnly1RunState(RunState::Stop);
}

void StainOnly1::setLightError()
{
//    MLights::GetInstance()->setStainOnly1AlarmStatus(AlarmStatus::Error);
}

void StainOnly1::setLightAlarm()
{
//    MLights::GetInstance()->setStainOnly1AlarmStatus(AlarmStatus::Alarm);
}

void StainOnly1::clearLightAlarm()
{
//    MLights::GetInstance()->setStainOnly1AlarmStatus(AlarmStatus::Good);
}
