#ifndef M_SMEAR_H
#define M_SMEAR_H

#include "module_base.h"

enum class SampleMode {
    Sample_Normal,
    Sample_Mini,
    Sample_NoMix
};

class MSampling;
class MStainCart;
class MSlideStore;
class MSmear : public DModuleBase
{
    Q_OBJECT
public:
    explicit MSmear(const QString &mid, QObject *parent = nullptr);
    void setMSample(MSampling *sample) { mSampling = sample; }
    void setStainCart(MStainCart *cart) { mStainCart = cart; }
    void setSlideBoxMgr(MSlideStore *slidebox) { mSlideBoxMgr = slidebox; }

    void setSmearParams(const QJsonObject &obj);
    void setSmearCartLoaded(bool isLoad, QSharedPointer<RtSlide> slide = nullptr);
    bool isCartLoaded() { return m_isCartLoaded; }

    void addNewSlideRequest(QSharedPointer<RtSlide> slide);

    bool cmd_Smear(const QJsonArray &arr);
    void setSmearZhightBasis(int base);
    void setSmearZhightBasisDB(int base);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    MSampling *mSampling = nullptr;
    MStainCart *mStainCart = nullptr;
    MSlideStore *mSlideBoxMgr = nullptr;

    bool m_isCartLoaded;

    void state_init();

    QJsonArray m_smearParamsArr;
    int z_hight_basis;

    /* smear sample */
    QList<QSharedPointer<RtSlide>> mRequestList;
    QSharedPointer<RtSlide> m_slide;

    QString m_api;

    QTimer *mTimer;
    enum class SmearState {
        Idle,
        WaitF_AddSample_Ready,

        Smear,
        WaitF_Smear_Done,

        Check_Stain_Cart_Idle,
        Send_To_Stain_Cart,
        WaitF_Send_To_Stain_Cart_Done,

        Check_All_Smear_Finish,
        SmearCart_Reset,
        WaitF_Reset_Done,

        Finish
    } s_smear;
    void smearProcess();

    /* wash blade */
    bool m_isCleanBlade;
    QSharedPointer<RtSlide> m_WashSlide;
    enum class WashState {
        Idle,

        Fill_Wash_Pool,
        WaitF_Fill_Wash_Done,

        Wash_Blade,
        WaitF_Wash_Blade_Done,

        Finish
    } s_wash;
    void washBladeProcess();

private slots:
    void onStateTimer_slot();
};

#endif // M_SMEAR_H
