#include "module_manager.h"

#include "cart/carts_manager.h"
#include "station/station_manager.h"
#include "exit/export_manager.h"
#include "entrance/m_import.h"
#include "process/m_boot_process.h"
#include "process/m_aging_test_process.h"
#include "settings/f_settings.h"
#include "device/rt_device_manager.h"
#include "sql/f_sql_database_manager.h"

DModuleManager *DModuleManager::GetInstance()
{
    static DModuleManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new DModuleManager();
    }
    return instance;
}

DModuleManager::DModuleManager(QObject *parent) : QObject(parent)
{
    _init();

    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (db) {
        db->deleteRecord("states");
        db->deleteRecord("samples");
    }
}

void DModuleManager::_init()
{
    mBoot = new MBootProcess(this);
    mBoot->setDeviceManager(RtDeviceManager::GetInstance());

    mAgingTest = new MAgingTestProcess(this);
    mAgingTest->setDeviceManager(RtDeviceManager::GetInstance());

    auto carts = CartsManager::GetInstance();
    auto exports = ExportManager::GetInstance();
    StationManager::GetInstance();

    mImport = new MImport(this);
    mImport->setDeviceManager(RtDeviceManager::GetInstance());

    carts->setExport1(exports->mExport1);
    carts->setExport2(exports->mExport2);

    mBoot->start();
}

void DModuleManager::start()
{
    if (FSettings::GetInstance()->isAgingTestMode()) {
        mAgingTest->start();
    } else {
        CartsManager::GetInstance()->start();
        StationManager::GetInstance()->start();
        ExportManager::GetInstance()->start();

        mImport->start();
    }
}

void DModuleManager::reset()
{
    CartsManager::GetInstance()->reset();
    StationManager::GetInstance()->reset();
    ExportManager::GetInstance()->reset();
    mImport->reset();

    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (db) {
        db->deleteRecord("states");
    }
}

void DModuleManager::stop()
{
    CartsManager::GetInstance()->stop();
    StationManager::GetInstance()->stop();
    ExportManager::GetInstance()->stop();
    mImport->stop();
}

