#ifndef M_SAMPLING_H
#define M_SAMPLING_H

#define With_Remove_Cap

#include "module_base.h"

enum SamplePos {
    Pos_Rack = 0,
    Pos_Emergency = 1,
    Pos_Normal = 2,
    Pos_Mini = 3        /* obsolete */
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
    bool isReceivable(bool isRet = false);
    bool receiveNewSample(QSharedPointer<RtSample> sample);
    bool isReceiveFinished() { return m_isReceiveFinished; }
    bool receiveEmergencySample(QSharedPointer<RtSample> sample);

    bool sendRecycleSample(QSharedPointer<RtSample> sample);
    bool isRecycleFinished() { return m_isRecycleFinished; }

    void setRetHatchTime(const QJsonObject &obj);
    void setRetHatchTime(int minute);

    /* command */
    bool cmd_GetNewSampleTube(bool isEmergency);
    bool cmd_ReturnTubeFrCart(bool isEmergency);
    bool cmd_CartToSamplePos(bool isUncapped);
    bool cmd_ReturnSampleTubeToOut(bool isUncapped);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

signals:
    void onTakeNewSample_signal(const QString &sid);

private:
    MSmear *mSmear;
    MSlideStore *mSlideBoxMgr;

    bool m_isUnited;
    int m_SamplingVolume;
    int m_AddSampleVolume;

    void state_init();

    QString m_api;

    QSharedPointer<RtSlide> mHandlingSlide;

    /* receive */
    QList<QSharedPointer<RtSample>> mUnsamplingList;

    QSharedPointer<RtSample> mSamplingSample;
    bool m_isReceiveFinished;

    /* Mix Sample */
    QSharedPointer<RtSample> mMixSample;
    enum class MixSample {
        Idle,

        Get_Sample,
        WaitF_Get_Sample_Done,

        Place_Tube_To_Mix,
        WaitF_Tube_To_Mix_Done,

        Mix_Sample,
        WaitF_Mix_Sample_Done,

        Finish
    } s_MixSample;
    void mixProcess();

    enum class Sampling {
        Idle,
        Receive_New_Sample,

        Get_New_SampleTube,
        WaitF_Get_New_SampleTube_Done,

        Put_Tube_In_Cart,
        WaitF_Put_Tube_In_Cart_Done,

        Get_Mixed_Tube,
        WaitF_Get_Mixed_Tube_Done,

        Cart_To_Sampling_Pos,
        WaitF_Cart_To_Sampling_Pos_Done,

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
    QString m_recycle_api;
    QSharedPointer<RtSample> mRecycleSample;
    bool m_isRecycleFinished;
    enum class RecycleTupe {
        Idle,

#ifdef With_Remove_Cap
        Move_Tube_To_Exit_Cmd,
        WaitF_Move_To_Exit_Done,
#endif

        Send_Recycle_Cmd,
        WaitF_Recycle_Done,

        Finish
    } s_Recycle;
    void ReturnTubeFrMixProcess();

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
