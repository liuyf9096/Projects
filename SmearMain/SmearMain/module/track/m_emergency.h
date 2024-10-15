#ifndef M_EMERGENCY_H
#define M_EMERGENCY_H

#include "module_base.h"
#include "sample/rt_sample.h"

class MSampling;
class MEmergency : public DModuleBase
{
    Q_OBJECT
public:
    explicit MEmergency(const QString &mid, QObject *parent = nullptr);

    void setSampling(MSampling *sampling) { mSampling = sampling; }
    void setCloseTime(int sec) { mWaitSec = sec; }

    void entranceCloset(quint64 id, const QJsonObject &obj);
    void startEmergency(quint64 id, const QJsonObject &obj);
    void handleRequest(const JPacket &packet);

    virtual void reset() override;

    bool Closet_Open();
    bool Closet_Close();
    bool isClosetOpenDone();
    bool isClosetCloseDone();

private:
    MSampling *mSampling = nullptr;
    bool m_isUnited;

    void state_init();

    void setEmergencySample(QSharedPointer<RtSample> sample, const QJsonObject &obj);

    bool m_isClosetOpen;

    QTimer *mOperateTimer;
    quint64 m_closetid;
    bool m_isClosetFinished;
    bool m_isOn;
    enum class OperateState {
        Idle,

        Operate_Closet,
        WaitF_Operate_Closet_Done,

        Finish,
        United_Finish
    } s_closet;

    QSharedPointer<RtSample> m_sample;
    QTimer *mEmergencyTimer;
    quint64 m_emergId;
    enum class EmergState {
        Idle,

        CloseCloset,
        WaitF_CloseCloset_Done,

        Send_Sample_To_Test,
        WaitF_Test_Finished,

        Recycle_Sample,
        WaitF_Recycle_Sample_Done,

        Open_Closet,
        WaitF_Open_Closet_Done,

#if 0
        WaitF_Timeout,
        CloseCloset2,
        WaitF_CloseCloset2_Done,
#endif

        Finish,
        United_Finish
    } s_emerg;
    int m_waitTime;
    int mWaitSec;

    bool m_isClosetOpenFinished;
    bool m_isClosetCloseFinished;

private slots:
    void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue);
    void onUnityMessageResult_slot(const JPacket &result, const JPacket &request);
    void onClosetTimeout_slot();
    void onEmergencyProcess_slot();
};

#endif // M_EMERGENCY_H
