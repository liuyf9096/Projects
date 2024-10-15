#include "m_export1.h"
#include "exception/exception_center.h"

MExport1::MExport1(QObject *parent)
    : MExport{"export1", "E1", parent}
{

}

void MExport1::cmd_UnloadRack()
{
    dev->track()->cmd_Exit_E1_Unload();
}

bool MExport1::isUnloadRack_Done()
{
    return dev->track()->isFuncDone("Exit_E1_Unload");
}

void MExport1::sendExitFullException()
{
    ExceptionCenter::GetInstance()->addException("mian", "Exit1_Full", Exception_Type::UserCode);
}

void MExport1::sendClearExitFullException()
{
    ExceptionCenter::GetInstance()->removeException("mian", "Exit1_Full");
}
