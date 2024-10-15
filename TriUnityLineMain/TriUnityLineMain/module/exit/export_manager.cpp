#include "export_manager.h"
#include "m_export1.h"
#include "m_export2.h"
#include "settings/f_settings.h"

ExportManager *ExportManager::GetInstance()
{
    static ExportManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new ExportManager();
    }
    return instance;
}

ExportManager::ExportManager(QObject *parent)
    : MManagerBase{"exports", parent}
{
    mExport1 = new MExport1(this);
    mExport2 = new MExport2(this);
    addModule(mExport1, mExport2);

    mExport1->setEnable(true);
    if (FSettings::GetInstance()->getExitCount() > 1) {
        mExport2->setEnable(true);
    } else {
        mExport2->setEnable(false);
    }
}
