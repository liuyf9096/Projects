#include "smear_manager.h"

SmearManager *SmearManager::GetInstance()
{
    static SmearManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new SmearManager();
    }
    return instance;
}

SmearManager::SmearManager(QObject *parent)
    : MManagerBase{"smear", parent}
{
    /* create */
    mSampling = new MSampling("sampling", this);
    mSlideStore = new MSlideStore("slidestore", this);
    mSmear = new MSmear("smear", this);

    /* settting */
    mSampling->setSlideBoxMgr(mSlideStore);
    mSampling->setSmear(mSmear);

    mSlideStore->setSmear(mSmear);

    mSmear->setSlideBoxMgr(mSlideStore);
    mSmear->setMSample(mSampling);

    addModule(mSampling, mSlideStore, mSmear);
}
