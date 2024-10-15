#ifndef M_UNITY_TASK_H
#define M_UNITY_TASK_H

#include "module_base.h"

class MSampling;
class DModuleManager;
class MUnityTask : public DModuleBase
{
    Q_OBJECT
public:
    explicit MUnityTask(QObject *parent = nullptr);

    void setSampling(MSampling *sampling) { mSampling = sampling; }
    void setManager(DModuleManager *manager) { mManager = manager; }

    void checkAvailable(quint64 id, const QJsonObject &obj);
    bool takeUpNewSample(quint64 id, const QJsonObject &obj);
    bool recycleSample(quint64 id, const QJsonObject &obj);
    void rotateTube(quint64 id);

private:
    MSampling *mSampling;
    DModuleManager *mManager;

    void state_init();

    QSharedPointer<RtSample> m_receiveSample;
    QSharedPointer<RtSample> m_recycleSample;

    QTimer *mTakeUpTimer;
    quint64 m_takeupId;
    bool m_isTakeupFinished;
    enum class TakeUpState {
        Idle,
        WaitF_TakeUp_Done,
        Finish
    } s_takeup;

    QTimer *mRecycleTimer;
    quint64 m_recycleId;
    bool m_isRecycleFinished;
    enum class RecycleState {
        Idle,
        Recycle_Sample,
        WaitF_Recycle_Done,
        Finish
    } s_recycle;

    QTimer *mRotateTimer;
    quint64 m_rotateId;
    bool m_isRotateFinished;
    enum class RotateState {
        Idle,
        Rotate_Tube,
        WaitF_Rotate_Done,
        Finish
    } s_rotate;

    QTimer *mCheckTimer;

    void sendResultMessage(quint64 id, const QJsonValue &res = true);
    void sendErrorMessage(quint64 id, const QString &msg);

private slots:
    void rotateTubeTimer_slot();
    void takeupTimer_slot();
    void recycleTimer_slot();
    void checkTimer_slot();
};

#endif // M_UNITY_TASK_H
