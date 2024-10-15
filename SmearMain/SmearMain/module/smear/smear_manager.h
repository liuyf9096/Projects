#ifndef SMEAR_MANAGER_H
#define SMEAR_MANAGER_H

#include "m_manager_base.h"
#include "m_smear.h"
#include "m_sampling.h"
#include "m_slidestore.h"

class SmearManager : public MManagerBase
{
    Q_OBJECT
public:
    static SmearManager *GetInstance();

    MSmear *mSmear;
    MSampling *mSampling;
    MSlideStore *mSlideStore;

private:
    explicit SmearManager(QObject *parent = nullptr);
};

#endif // SMEAR_MANAGER_H
