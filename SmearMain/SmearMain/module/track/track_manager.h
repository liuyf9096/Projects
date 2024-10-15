#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

#include "m_manager_base.h"

#include "m_import.h"
#include "m_export.h"
#include "m_emergency.h"
#include "cart/carts_manager.h"

class TrackManager : public MManagerBase
{
    Q_OBJECT
public:
    static TrackManager *GetInstance();

    MImport *mImport;
    MExport *mExport;
    MEmergency *mEmergency;
    CartsManager *mCarts;

    void abort();
    void stopDetectNewRack();

private:
    explicit TrackManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(TrackManager)
};

#endif // TRACK_MANAGER_H
