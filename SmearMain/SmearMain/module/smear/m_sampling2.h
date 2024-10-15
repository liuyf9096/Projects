#ifndef M_SAMPLING_H
#define M_SAMPLING_H

#include "module_base.h"

/* Sample With No Cap 2022-11-07 */

enum SamplePos {
    Pos_Rack = 0,
    Pos_Emergency = 1,
    Pos_Normal = 2,
    Pos_Mini = 3
};

class MCart;
class MSmear;
class MSlideStore;
class MSampling : public DModuleBase
{
    Q_OBJECT
public:
    explicit MSampling(const QString &mid, QObject *parent = nullptr);
    void setSmear(MSmear *smear) { mSmear = smear; }
    void setSlideBoxMgr(MSlideStore *slidebox) { mSlideBoxMgr = slidebox; }

    /* setting */
    void setSamplingVolume(int volume) { m_SamplingVolume = volume; }
    void setAddSampleVolume(int volume);

    /* start */
    bool isReceivable(QSharedPointer<RtSample> sample = nullptr);
    bool receiveNewSample(QSharedPointer<RtSample> sample);
    bool isReceiveFinished() { return m_isReceiveFinished; }
    bool receiveEmergencySample(QSharedPointer<RtSample> sample);

    bool sendRecycleSample(QSharedPointer<RtSample> sample);
    bool isRecycleFinished() { return m_isRecycleFinished; }

    void setRetHatchTime(const QJsonObject &obj);
    void setRetHatchTime(int minute);

    /* command */
    bool cmd_GetNewSampleTube(bool isEmergency);
    bool cmd_MixNewSampleTube(QSharedPointer<RtSample> sample);
    bool cmd_NeedleTakeSample(QSharedPointer<RtSample> sample, int volume = 200);
    bool cmd_ReturnSampleTube(bool isEmergency, bool isMiniBlood);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

signals:
    void onTakeNewSample_signal(const QString &sid);

private:
    MSmear *mSmear;
    MSlideStore *mSlideBoxMgr;

    int m_SamplingVolume;
    int m_AddSampleVolume;

    void state_init();

    QString m_api;

    QSharedPointer<RtSlide> mHandlingSlide;

    /* receive */
    QList<QSharedPointer<RtSample>> mUnsamplingList;
    QSharedPointer<RtSample> mHandlingSample;
    bool m_isReceiveFinished;
    enum class Sampling {
        Idle,
        Receive_New_Sample,

        Get_New_SampleTube,
        WaitF_Get_New_SampleTube_Done,

        Send_New_SampleTube,
        WaitF_Mix_New_SampleTube_Done,

        Sampling,
        WaitF_Sampling_Done,

        Send_To_Add_Sample,

        Finish
    } s_Sampling;
    void samplingProcess();

    /* Add Sample */
    QString m_cleanApi;
    bool m_isNeedleClean;
    QSharedPointer<RtSample> mAddSample;
    enum class AddSample {
        Idle,

        Prepare_Sampling,
        WaitF_Prepare_Sampling_Done,

        Ret_Mix_Sample,
        WaitF_Ret_Mix_Done,

        WaitF_NewSlide_Ready,

        Send_Add_Sample_Cmd,
        WaitF_Add_Sample_Done,

        Clean_Needles,
        WaitF_Clean_Needles_Done,

        Finish
    } s_AddSample;
    void addSampleProcess();

    /* recycle */
    bool m_isRecycleFinished;
    enum class RecycleTupe {
        Idle,
        Send_Recycle_Cmd,
        WaitF_Recycle_Done,
        Finish
    } s_Recycle;
    void recycleTubeProcess();

    QTimer *mStateTimer;

    void startHatch();
    bool m_handle_ret;
    bool m_hatchFinished;
    QTimer *mHatchTimer;

    bool m_ret_lock;
    QSharedPointer<RtSample> mRetSample;
    enum class RetState {
        Idle,

        Start_Hatch,
        WaitF_Hatch_Done,

        Lock_Other_Process,
        WaitF_Lock_Other_Process_Done,

        Take_Ret_Sample,
        WaitF_Take_Ret_Sample_Done,

        Add_Sample,
        WaitF_Add_Sample_Done,

        Finish
    } s_ret;
    void hatchRetProcess();

private slots:
    void onStateTimer_slot();
    void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue);
};

#endif // M_SAMPLING_H
