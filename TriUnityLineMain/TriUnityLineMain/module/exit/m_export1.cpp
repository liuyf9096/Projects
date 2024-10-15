#include "m_export1.h"

MExport1::MExport1(QObject *parent)
    : MExport{"export1", "E1", parent}
{

}

void MExport1::cmd_UnloadRack()
{
    dev->track1()->cmd_Exit_E1_Unload();
}

bool MExport1::isUnloadRack_Done()
{
    return dev->track1()->isFuncDone("Exit_E1_Unload");
}
