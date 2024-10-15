#include "m_manager_base.h"
#include "module_base.h"
#include "device/rt_device_manager.h"

MManagerBase::MManagerBase(const QString &managerName, QObject *parent)
    : QObject{parent}
    , mManagerName(managerName)
{
}

void MManagerBase::addModule(DModuleBase *module)
{
    Q_ASSERT(module);

    module->setDeviceManager(RtDeviceManager::GetInstance());
    module->mGroup = mManagerName;
    connect(module, &DModuleBase::onDisplayMessage_signal,
            this, &MManagerBase::onDisplayMessage_signal);

    ModuleMap.insert(module->mid(), module);
}

void MManagerBase::addModule(DModuleBase *m1, DModuleBase *m2)
{
    addModule(m1);
    addModule(m2);
}

void MManagerBase::addModule(DModuleBase *m1, DModuleBase *m2, DModuleBase *m3)
{
    addModule(m1);
    addModule(m2);
    addModule(m3);
}

void MManagerBase::start()
{
    for (DModuleBase* module : qAsConst(ModuleMap)) {
        module->start();
    }
}

void MManagerBase::reset()
{
    for (DModuleBase* module : qAsConst(ModuleMap)) {
        module->reset();
    }
}

void MManagerBase::stop()
{
    for (DModuleBase* module : qAsConst(ModuleMap)) {
        module->stop();
    }
}


