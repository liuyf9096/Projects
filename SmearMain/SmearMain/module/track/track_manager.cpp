#include "track_manager.h"
#include "cart/m_cart.h"

TrackManager *TrackManager::GetInstance()
{
    static TrackManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new TrackManager();
    }
    return instance;
}

TrackManager::TrackManager(QObject *parent)
    : MManagerBase{"track", parent}
{
    mImport = new MImport("import", this);
    mExport = new MExport("export", this);
    mEmergency = new MEmergency("emergency", this);

    mCarts = CartsManager::GetInstance();
    mCarts->setExport(mExport);

    addModule(mImport, mExport);
    addModule(mCarts->mCart1, mCarts->mCart2);
    addModule(mEmergency);
}

void TrackManager::abort()
{
    stopDetectNewRack();
    mCarts->abort();
}

void TrackManager::stopDetectNewRack()
{
    mImport->stopReceiveNewRack();
}
