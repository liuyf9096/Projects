#include "m_export2.h"

MExport2::MExport2(QObject *parent)
    : MExport{"export2", "E2", parent}
{

}

void MExport2::cmd_UnloadRack()
{
    dev->track2()->cmd_Exit_E2_Unload();
}

bool MExport2::isUnloadRack_Done()
{
    return dev->track2()->isFuncDone("Exit_E2_Unload");
}
