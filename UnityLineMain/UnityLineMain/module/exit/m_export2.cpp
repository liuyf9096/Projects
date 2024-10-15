#include "m_export2.h"
#include "exception/exception_center.h"

MExport2::MExport2(QObject *parent)
    : MExport{"export2", "E2", parent}
{

}

void MExport2::cmd_UnloadRack()
{
    dev->track()->cmd_Exit_E2_Unload();
}

bool MExport2::isUnloadRack_Done()
{
    return dev->track()->isFuncDone("Exit_E2_Unload");
}

void MExport2::sendExitFullException()
{
    ExceptionCenter::GetInstance()->addException("mian", "Exit2_Full", Exception_Type::UserCode);
}

void MExport2::sendClearExitFullException()
{
    ExceptionCenter::GetInstance()->removeException("mian", "Exit2_Full");
}
