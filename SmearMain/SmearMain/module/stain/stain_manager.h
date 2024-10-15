#ifndef STAIN_MANAGER_H
#define STAIN_MANAGER_H

#include "m_manager_base.h"

#include "m_stain_cart.h"
#include "gripper/gripper_manager.h"
#include "slot/slots_manager.h"
#include "recyclebox/m_recyclebox_mgr.h"
#include "solution/solution_infuser.h"
#include "solution/solution_drainer.h"

#include "entrance/stain_import.h"
#include "entrance/stainonly1.h"
#include "entrance/stainonly2.h"

class QTimer;
class StainManager : public MManagerBase
{
    Q_OBJECT
public:
    static StainManager *GetInstance();

    void setStainParams(const QJsonObject &obj);

    MStainCart *mStainCart;
    GripperManager *mGrippers;
    SlotsManager *mSlotManager;
    MRecycleBoxMgr *mRecycleBoxes;
    SolutionInfuser *mInfuser;
    SolutionDrainer *mDrainer;

    StainImport *pImport;
    StainOnly1 *pStainOnly1;
    StainOnly2 *pStainOnly2;

    void setCheckTimerInterval(int sec);
    void startCheckTimer(bool firstRun);
    void needCheckImmediately();

    void startStainOnlyProcess(quint64 id, const QJsonObject &obj);
    bool addStainOnlyRequest(quint64 id, int index, const QJsonArray &arr);
    void cancelStainOnlyRequest();

    void takingOutSample(const QString &pid, const QString &sid, int pos = -1);
    void takeOutSample(const QString &group_id, const QString &sid, int pos = -1);

    virtual void stop() override;

signals:
    void onGripperRequest_signal(const QJsonObject &obj);

private:
    explicit StainManager(QObject *parent = nullptr);
    QTimer *mCheckNewTimer;

private slots:
    void onCheckNewTimer_slot();
    void handleSlotTimeout_slot(const QString &from_groupid, int from_pos, const QString &sid);
};

#endif // STAIN_MANAGER_H
