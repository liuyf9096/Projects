#include "m_smear.h"
#include "sample/rt_sample.h"
#include "m_slidestore.h"
#include "m_sampling.h"
#include "stain/m_stain_cart.h"
#include "f_common.h"

MSmear::MSmear(const QString &mid, QObject *parent)
    : DModuleBase(mid, "smear", parent)
{
    mTimer = new QTimer(this);
    mTimer->setInterval(100);
    connect(mTimer, &QTimer::timeout, this, &MSmear::onStateTimer_slot);

    z_hight_basis = 0;
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("smearparams");
    if (db) {
        QJsonArray arr = db->selectRecord("basis", QJsonObject({{"params", "z_hight"}}));
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            int value = obj.value("value").toInt();
            setSmearZhightBasis(value);
        }
    }

    state_init();
}

void MSmear::state_init()
{
    m_isCartLoaded = false;
    m_isCleanBlade = true;

    m_slide = nullptr;
    m_WashSlide = nullptr;

    s_smear = SmearState::Idle;
    s_wash = WashState::Idle;
}

void MSmear::setSmearParams(const QJsonObject &obj)
{
    int smearHeadZ = obj.value("smearHeadZ").toInt();
    int sampleExtendPos = obj.value("sampleExtendPos").toInt();
    int sampleExtendTime = obj.value("sampleExtendTime").toInt();
    int smearStartSpeed = obj.value("smearStartSpeed").toInt();
    int smearMaxSpeed = obj.value("smearMaxSpeed").toInt();
    int smearRange = obj.value("smearRange").toInt();
    int smearZWaitTime = obj.value("smearZWaitTime").toInt();
    int smearZStartTime = obj.value("smearZStartTime").toInt();
    int smearZMaxTime = obj.value("smearZMaxTime").toInt();
    int smearZRange = obj.value("smearZRange").toInt();

    QJsonArray arr;
    arr.append(smearHeadZ);
    arr.append(sampleExtendPos);
    arr.append(sampleExtendTime);
    arr.append(smearStartSpeed);
    arr.append(smearMaxSpeed);
    arr.append(smearRange);
    arr.append(smearZWaitTime);
    arr.append(smearZStartTime);
    arr.append(smearZMaxTime);
    arr.append(smearZRange);

    m_smearParamsArr = arr;
    qDebug() << "Use UserParams:" << m_smearParamsArr;
}

void MSmear::setSmearCartLoaded(bool isLoad, QSharedPointer<RtSlide> slide)
{
    m_isCartLoaded = isLoad;
    if (isLoad == true) {
        recordModuleSlideState("smear_cart", "doing_slide", slide);
    } else {
        recordModuleSlideState("smear_cart", "doing_slide", nullptr);
    }
}

void MSmear::addNewSlideRequest(QSharedPointer<RtSlide> slide)
{
    Q_ASSERT(slide);
    qDebug() << "Smear add new slide:" << slide->slide_id();
    mRequestList.append(slide);
}

bool MSmear::cmd_Smear(const QJsonArray &arr)
{
    if (arr.count() >= 10) {
        int arg1 = arr.at(0).toInt() + z_hight_basis;
        int arg2 = arr.at(1).toInt();
        int arg3 = arr.at(2).toInt();
        int arg4 = arr.at(3).toInt();
        int arg5 = arr.at(4).toInt();
#if 0
        int arg6 = arr.at(5).toInt();
        int arg7 = arr.at(6).toInt();
        int arg8 = arr.at(7).toInt();
        int arg9 = arr.at(8).toInt();
        int arg10 = arr.at(9).toInt();
        bool ok = dev->smear()->cmd_Smear(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
#else
        bool ok = dev->smear()->cmd_Smear2(arg1, arg2, arg3, arg4, arg5);
#endif
        return ok;
    }
    return false;
}

void MSmear::setSmearZhightBasis(int base)
{
    z_hight_basis = base;
    qDebug() << "setSmearZhightBasis" << base;
}

void MSmear::setSmearZhightBasisDB(int base)
{
    setSmearZhightBasis(base);

    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("smearparams");
    if (db) {
        db->updateRecord("basis", {{"value", base}}, QJsonObject({{"params", "z_hight"}}));
    }
}

void MSmear::start()
{
    mTimer->start();
}

void MSmear::reset()
{
    state_init();
}

void MSmear::stop()
{
    mTimer->stop();
}

void MSmear::onStateTimer_slot()
{
    smearProcess();
    washBladeProcess();
}

void MSmear::smearProcess()
{
    switch (s_smear) {
    case SmearState::Idle:
        if (m_slide != nullptr)
        {
            if (m_slide->isSmearEnable()) {
                if (m_slide->getStatus(SmearStatus::Smear) == 0) {
                    if (m_WashSlide == nullptr) {
                        m_WashSlide = m_slide;
                        logProcess("Smear", 1, "Start", m_slide->slide_id());
                        s_smear = SmearState::WaitF_AddSample_Ready;
                    }
                }
            } else {
                s_smear = SmearState::Check_Stain_Cart_Idle;
            }
        } else if (mRequestList.isEmpty() == false) {
            m_slide = mRequestList.takeFirst();
        }
        break;

        /* 1. Wait Sample Slide Prepare Ready */
    case SmearState::WaitF_AddSample_Ready:
        if (m_slide->getStatus(SmearStatus::Add_Sample) == 1) {
            s_smear = SmearState::Smear;
        }
        break;

        /* 2. Smear */
    case SmearState::Smear:
        if (m_isCleanBlade == true) {
            RtSampleManager::GetInstance()->sendUISlideStatus(m_slide, SlideProcessState::Smearing);
            m_slide->setStatus(SmearStatus::Smear, 0);

            auto sample = m_slide->sample();
            Q_ASSERT(sample);
            cmd_Smear(sample->smearParams());

            m_isCleanBlade = false;
            s_smear = SmearState::WaitF_Smear_Done;
        }
        break;
    case SmearState::WaitF_Smear_Done:
        if (dev->smear()->isFuncDone("Smear") == true) {
            m_slide->setStatus(SmearStatus::Smear, 1);
            s_smear = SmearState::Check_Stain_Cart_Idle;
        }
        break;

        /* 3. Send To Stain Cart */
    case SmearState::Check_Stain_Cart_Idle:
        if (mStainCart->isIdle()) {
            m_slide->setStatus(SmearStatus::Send_To_StainCart, 0);
            s_smear = SmearState::Send_To_Stain_Cart;
        }
        break;

    case SmearState::Send_To_Stain_Cart:
        dev->sample()->cmd_StainCart_LoadSlide();
        s_smear = SmearState::WaitF_Send_To_Stain_Cart_Done;
        break;
    case SmearState::WaitF_Send_To_Stain_Cart_Done:
        if (dev->sample()->isFuncDone("StainCart_LoadSlide")) {
            m_slide->setStatus(SmearStatus::Send_To_StainCart, 1);
            setSmearCartLoaded(false);
            mStainCart->receiveNewSlide(m_slide);

            if (m_slide->isSmearEnable()) {
                RtSampleManager::GetInstance()->sendUISlideStatus(m_slide, SlideProcessState::SmearFinish);
            }

            logProcess("Smear", 2, "Smear Finish", m_slide->slide_id());
            m_slide = nullptr;

            s_smear = SmearState::Check_All_Smear_Finish;
        }
        break;

    case SmearState::Check_All_Smear_Finish:
        if (RtSampleManager::GetInstance()->allSampleSmearFinished()) {
            s_smear = SmearState::SmearCart_Reset;
        } else {
            s_smear = SmearState::Finish;
        }
        break;
    case SmearState::SmearCart_Reset:
    {
        bool ok = dev->smear()->cmd_SmearCart_Reset();
        if (ok) {
            s_smear = SmearState::WaitF_Reset_Done;
        }
    }
        break;
    case SmearState::WaitF_Reset_Done:
        if (dev->smear()->isFuncDone("SmearCart_Reset")) {
            s_smear = SmearState::Finish;
        }
        break;

    case SmearState::Finish:
        logProcess("Smear", 99, "Finish");
        s_smear = SmearState::Idle;
        break;
    }
}

void MSmear::washBladeProcess()
{
    switch (s_wash) {
    case WashState::Idle:
        if (m_WashSlide != nullptr) {
            logProcess("WashBlade", 1, "Start", m_WashSlide->slide_id());
            s_wash = WashState::Fill_Wash_Pool;
        }
        break;

        /* 1. Fill Wash Pool */
    case WashState::Fill_Wash_Pool:
    {
        bool ok = dev->smear()->cmd_FillWashPool();
        if (ok) {
            s_wash = WashState::WaitF_Fill_Wash_Done;
        }
    }
        break;
    case WashState::WaitF_Fill_Wash_Done:
        if (dev->smear()->isFuncDone("FillWashPool")) {
            s_wash = WashState::Wash_Blade;
        }
        break;

        /* 2. Wash Blade */
    case WashState::Wash_Blade:
        if (m_WashSlide->getStatus(SmearStatus::Smear) == 1) {
            m_WashSlide->setStatus(SmearStatus::Wash_Blade, 0);
            dev->smear()->cmd_CleanSmearBlade();
            s_wash = WashState::WaitF_Wash_Blade_Done;
        }
        break;
    case WashState::WaitF_Wash_Blade_Done:
        if (dev->smear()->isFuncDone("CleanSmearBlade")) {
            m_WashSlide->setStatus(SmearStatus::Wash_Blade, 1);
            m_isCleanBlade = true;
            s_wash = WashState::Finish;
        }
        break;

    case WashState::Finish:
        logProcess("WashBlade", 99, "Finish", m_WashSlide->slide_id());
        m_WashSlide = nullptr;
        s_wash = WashState::Idle;
        break;
    }
}
