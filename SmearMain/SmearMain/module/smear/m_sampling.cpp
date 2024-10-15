#include "m_sampling.h"
#include "m_slidestore.h"
#include "m_smear.h"
#include "record/rt_machine_record.h"
#include "sql/f_sql_database_manager.h"
#include "f_common.h"

//#define Ret

MSampling::MSampling(const QString &mid, QObject *parent)
    : DModuleBase(mid, "sampling", parent)
{
    m_SamplingVolume = 25;
    m_AddSampleVolume = 3;

    m_isUnited = FCommon::GetInstance()->isUnited();
    mStateTimer = new QTimer(this);
    mStateTimer->setInterval(100);
    connect(mStateTimer, &QTimer::timeout,
            this, &MSampling::onStateTimer_slot);

    mHatchTimer = new QTimer(this);
    mHatchTimer->setInterval(2 * 60 * 1000);
    mHatchTimer->setSingleShot(true);
    connect(mHatchTimer, &QTimer::timeout, this, [=](){
        m_hatchFinished = true;
    });

    state_init();

    auto sample = RtDeviceManager::GetInstance()->sample();
    connect(sample, &DSample::onFunctionFinished_signal,
            this, &MSampling::onFunctionFinished_slot);
}

void MSampling::setAddSampleVolume(int volume)
{
    m_AddSampleVolume = volume;
    qDebug() << "set AddSample Volume:" << volume;
}

void MSampling::state_init()
{
    m_ret_lock = false;
    m_handle_ret = false;

    mUnsamplingList.clear();
    m_isRecycleFinished = true;
    m_isReceiveFinished = true;
    m_hatchFinished = true;

    m_isNeedleClean = true;

    mMixSample = nullptr;
    mHandlingSlide = nullptr;
    mSamplingSample = nullptr;
    mAddSample = nullptr;
    mRetSample = nullptr;

    s_MixSample = MixSample::Idle;
    s_Sampling = Sampling::Idle;
    s_AddSample = AddSample::Idle;
    s_Recycle = RecycleTupe::Idle;
    s_ret = RetState::Idle;
}

void MSampling::start()
{
    mStateTimer->start();
    DModuleBase::start();
}

void MSampling::reset()
{
    state_init();
    DModuleBase::reset();
}

void MSampling::stop()
{
    mStateTimer->stop();
    DModuleBase::stop();
}

/* auto sampling */
bool MSampling::isReceivable(bool isRet)
{
    if (m_ret_lock == true) {
        return false;
    }

    if (isRet) {
        if (m_handle_ret == true) {
            return false;
        }
    }

    if (mUnsamplingList.isEmpty()) {
        if (mMixSample == nullptr) {
            return true;
        }
    }

    return false;
}

bool MSampling::receiveNewSample(QSharedPointer<RtSample> sample)
{
    if (sample == nullptr) {
        return false;
    }
    if (sample->isRet()) {
        return false;
    }
    if (isReceivable(sample->isRet())) {
        mMixSample = sample;
        m_isReceiveFinished = false;
        return true;
    }
    return false;
}

/* Action: No-UnCapping AND No-Mix */
bool MSampling::receiveEmergencySample(QSharedPointer<RtSample> sample)
{
    if (m_ret_lock == true) {
        return false;
    }
    if (sample->isEmergency()) {
        sample->setIsNeedMix(false);

        if (sample->isRet()) {
            if (m_handle_ret == false) {
                m_handle_ret = true;
                mUnsamplingList.append(sample);
                m_isReceiveFinished = false;
                return true;
            }
        } else {
            mUnsamplingList.append(sample);
            m_isReceiveFinished = false;
            return true;
        }
    }
    return false;
}

bool MSampling::sendRecycleSample(QSharedPointer<RtSample> sample)
{
    if (sample == nullptr) {
        return false;
    }
    if (mSamplingSample != nullptr && mSamplingSample->sid() == sample->sid()) {
        if (mSamplingSample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
            m_isRecycleFinished = false;
            return true;
        }
    }
    return false;
}

void MSampling::setRetHatchTime(const QJsonObject &obj)
{
    if (obj.contains("Ret")) {
        QJsonObject retObj = obj.value("Ret").toObject();
        int hatchTime = retObj.value("hatchTime").toInt();
        QJsonObject ratioObj = retObj.value("ratio").toObject();

        int a = ratioObj.value("a").toInt();
        int b = ratioObj.value("b").toInt();
        qInfo() << "ret hatch ratio:" << a << ":" << b;

        setRetHatchTime(hatchTime);
    }
}

void MSampling::setRetHatchTime(int minute)
{
    mHatchTimer->setInterval(minute * 60 * 1000);
    qInfo() << "Set Ret Hatch Time:" << minute << "min";
}

bool MSampling::cmd_GetNewSampleTube(bool isEmergency)
{
    if (isEmergency) {
        return dev->sample()->cmd_GetNewTube(Pos_Emergency);
    } else {
        return dev->sample()->cmd_GetNewTube(Pos_Rack);
    }
}

bool MSampling::cmd_ReturnTubeFrCart(bool isEmergency)
{
    if (isEmergency) {
        return dev->sample()->cmd_ReturnTubeFrCart(Pos_Emergency);
    } else {
        return dev->sample()->cmd_ReturnTubeFrCart(Pos_Rack);
    }
}

bool MSampling::cmd_CartToSamplePos(bool isUncapped)
{
    if (isUncapped == true) {
        m_api = "CartToSamplePos";
        return dev->sample()->cmd_CartToSamplePos();
    } else {
        /* record */
        recordModuleSampleState("cart", false);

        m_api = "CartToSamplePos_UnCapping";
        return dev->sample()->cmd_CartToSamplePos_UnCapping();
    }
}

bool MSampling::cmd_ReturnSampleTubeToOut(bool isUncapped)
{
    if (isUncapped == true) {
        m_recycle_api = "CartToExitPos";
        return dev->sample()->cmd_CartToExitPos();
    } else {
        m_recycle_api = "CartToExitPos_Capping";
        return dev->sample()->cmd_CartToExitPos_Capping();
    }
}

void MSampling::onStateTimer_slot()
{
    if (dev->sample()->isResetOk() == false) {
        return;
    }

    mixProcess();
    samplingProcess();
    addSampleProcess();
    ReturnTubeFrMixProcess();
    hatchRetProcess();
}

void MSampling::mixProcess()
{
    switch (s_MixSample) {
    case MixSample::Idle:
        if (mMixSample != nullptr) {
            logProcess("Mix", 1, "Receive Sample Tube To Mix", mMixSample->sid());

            mMixSample->setStatus(SampleStatus::Mixing, 0);
            s_MixSample = MixSample::Get_Sample;
        }
        break;

    case MixSample::Get_Sample:
    {
        bool ok = cmd_GetNewSampleTube(mMixSample->isEmergency());
        if (ok) {
            /* record */
            recordModuleSampleState("gripper", "doing_sample", mMixSample);

            s_MixSample = MixSample::WaitF_Get_Sample_Done;
        }
    }
        break;
    case MixSample::WaitF_Get_Sample_Done:
        if (dev->sample()->isFuncDone("GetNewTube")) {
            m_isReceiveFinished = true;
            s_MixSample = MixSample::Place_Tube_To_Mix;
        }
        break;

    case MixSample::Place_Tube_To_Mix:
    {
        bool ok = dev->sample()->cmd_TubeFrGripToMix();
        if (ok) {
            s_MixSample = MixSample::WaitF_Tube_To_Mix_Done;
        }
    }
        break;
    case MixSample::WaitF_Tube_To_Mix_Done:
        if (dev->sample()->isFuncDone("TubeFrGripToMix")) {

            /* record */
            recordModuleSampleState("gripper", "doing_sample", nullptr);
            recordModuleSampleState("mix", "doing_sample", mMixSample);

            logProcess("Mix", 2, "Mix Tube", mMixSample->sid());
            s_MixSample = MixSample::Mix_Sample;
        }
        break;

    case MixSample::Mix_Sample:
        dev->sample()->cmd_MixTube();
        s_MixSample = MixSample::WaitF_Mix_Sample_Done;
        break;
    case MixSample::WaitF_Mix_Sample_Done:
        if (dev->sample()->isFuncDone("MixTube")) {
            mMixSample->setStatus(SampleStatus::Mixing, 1);
            s_MixSample = MixSample::Finish;
        }
        break;

    case MixSample::Finish:
        if (mMixSample == nullptr) {
            logProcess("Mix", 99, "Finish");
            s_MixSample = MixSample::Idle;
        }
        break;
    }
}

void MSampling::samplingProcess()
{
    switch (s_Sampling) {
    case Sampling::Idle:
        if (mSamplingSample == nullptr) {
            if (mMixSample != nullptr) {
                if (mMixSample->getStatus(SampleStatus::Mixing) == 1) {
                    mSamplingSample = mMixSample;
                }
            } else {
                if (mUnsamplingList.isEmpty() == false) {
                    mSamplingSample = mUnsamplingList.takeFirst();
                }
            }
            if (mSamplingSample != nullptr) {
                mSamplingSample->setStatus(SampleStatus::Test_Finished, 0);
                qDebug() << "slide list:" << mSamplingSample->slides;
                logProcess("Sampling", 1, "Receive Sample Request", mSamplingSample->sid());
                s_Sampling = Sampling::Receive_New_Sample;
            }
        }
        break;

    case Sampling::Receive_New_Sample:
        if (mSamplingSample->slides.isEmpty() == false)
        {
            if (mSamplingSample->isRet() == false) {
                /* prepare slides and smear blade pool fill */
                for (int i = 0; i < mSamplingSample->slides.count(); ++i) {
                    mSlideBoxMgr->addNewSlideRequest(mSamplingSample->slides.at(i));
                    mSmear->addNewSlideRequest(mSamplingSample->slides.at(i));
                }
            }

            emit onTakeNewSample_signal(mSamplingSample->sid());
            logProcess("Sampling", 2, "Receive Sample Tube Request", mSamplingSample->sid());

            if (mSamplingSample->isNeedMix()) {
                s_Sampling = Sampling::Get_Mixed_Tube;
            } else {
                s_Sampling = Sampling::Get_New_SampleTube;
            }
        } else {
            qCritical() << "slides.isEmpty";
        }
        break;

        /* 1.1 Get Mixed New Tube */
    case Sampling::Get_Mixed_Tube:
    {
        bool ok = dev->sample()->cmd_TubeFrMixToCart();
        if (ok) {
            logProcess("Sampling", 3, "Move Tube To Cart", mSamplingSample->sid());
            s_Sampling = Sampling::WaitF_Get_Mixed_Tube_Done;
        }
    }
        break;
    case Sampling::WaitF_Get_Mixed_Tube_Done:
        if (dev->sample()->isFuncDone("TubeFrMixToCart")) {
            mMixSample = nullptr;

            /* record */
            recordModuleSampleState("mix", "doing_sample", nullptr);
            recordModuleSampleState("cart", "doing_sample", mSamplingSample);

            s_Sampling = Sampling::Cart_To_Sampling_Pos;
        }
        break;

        /* 1.2 Get New Tube (Emergency) */
    case Sampling::Get_New_SampleTube:        
        cmd_GetNewSampleTube(mSamplingSample->isEmergency());
        logProcess("Sampling", 3, "Get New Tube", mSamplingSample->sid());

        /* record */
        recordModuleSampleState("gripper", mSamplingSample);

        s_Sampling = Sampling::WaitF_Get_New_SampleTube_Done;
        break;

    case Sampling::WaitF_Get_New_SampleTube_Done:
        if (dev->sample()->isFuncDone("GetNewTube")) {
            mSamplingSample->setStatus(SampleStatus::Send_To_Test, 1);
            m_isReceiveFinished = true;
            s_Sampling = Sampling::Put_Tube_In_Cart;
        }
        break;

        /* 2.1 Put Tube In Cart (Emergency) */
    case Sampling::Put_Tube_In_Cart:
    {
        bool ok = dev->sample()->cmd_TubeFrGripToCart();
        if (ok) {
            s_Sampling = Sampling::WaitF_Put_Tube_In_Cart_Done;
        }
    }
        break;
    case Sampling::WaitF_Put_Tube_In_Cart_Done:
        if (dev->sample()->isFuncDone("TubeFrGripToCart")) {

            /* record */
            recordModuleSampleState("gripper", "doing_sample", nullptr);
            recordModuleSampleState("cart", "doing_sample", mSamplingSample);

            s_Sampling = Sampling::Cart_To_Sampling_Pos;
        }
        break;

        /* 2. Move Cart to Sampling Pos */
    case Sampling::Cart_To_Sampling_Pos:
        cmd_CartToSamplePos(mSamplingSample->isMiniBlood());

        s_Sampling = Sampling::WaitF_Cart_To_Sampling_Pos_Done;
        break;
    case Sampling::WaitF_Cart_To_Sampling_Pos_Done:
        if (dev->sample()->isFuncDone(m_api)) {
            s_Sampling = Sampling::Sampling;
        }
        break;

        /* 3. Sampling */
    case Sampling::Sampling:
        if (m_isNeedleClean == true) {
            m_isNeedleClean = false;

            dev->sample()->cmd_NeedleAdd_TakeSample(m_SamplingVolume);
            mSamplingSample->setStatus(SampleStatus::Sampling, 0);
            RtSampleManager::GetInstance()->sendUISampleStatus(mSamplingSample, SampleProcessState::Sampling);

#ifdef Ret
            if (mSamplingSample->isRet()) {
                dev->sample()->cmd_PoolRet_Fill();
            }
#endif
            logProcess("Sampling", 4, "Sampling", mSamplingSample->sid());
            s_Sampling = Sampling::WaitF_Sampling_Done;
        }
        break;
    case Sampling::WaitF_Sampling_Done:
        if (dev->sample()->isFuncDone("NeedleAdd_TakeSample")) {
            mSamplingSample->setStatus(SampleStatus::Sampling, 1);
            mSamplingSample->setStatus(SampleStatus::Test_Finished, 1);

            s_Sampling = Sampling::Send_To_Add_Sample;
        }
        break;

    case Sampling::Send_To_Add_Sample:
        if (mAddSample == nullptr) {
            mAddSample = mSamplingSample;
            mRecycleSample = mSamplingSample;
            logProcess("Sampling", 99, "Finish", mSamplingSample->sid());
            s_Sampling = Sampling::Finish;
        }
        break;

    case Sampling::Finish:
        break;

    default:
        break;
    }
}

void MSampling::addSampleProcess()
{
    switch (s_AddSample) {
    case AddSample::Idle:
        if (mAddSample != nullptr) {
            if (mAddSample->getStatus(SampleStatus::Add_Sample) == 0) {
                if (mAddSample->getStatus(SampleStatus::Sampling) == 1) {
                    logProcess("AddSample", 1, "Start", mAddSample->sid());
                    mAddSample->setStatus(SampleStatus::Prepare_Blood, 1);
                    s_AddSample = AddSample::WaitF_NewSlide_Ready;
                }
            }
        }
        break;

#ifdef Ret
        /* Ret Mix Sample */
    case AddSample::Ret_Mix_Sample:
        if (dev->sample()->isFuncDone("PoolRet_Fill")) {
            dev->sample()->cmd_RetMixSample(50, 40);
            s_AddSample = AddSample::WaitF_Ret_Mix_Done;
        }
        break;
    case AddSample::WaitF_Ret_Mix_Done:
        if (dev->sample()->isFuncDone("RetMixSample")) {
            mRetSample = mAddSample; //start hatch
            s_AddSample = AddSample::Clean_Needles;
        }
        break;
#endif

        /* 1. Wait for New Slide Ready */
    case AddSample::WaitF_NewSlide_Ready:
        if (mHandlingSlide != nullptr) {
            if (mHandlingSlide->getStatus(SmearStatus::WaitF_Add_Blood) == 1) {
                mHandlingSlide->setStatus(SmearStatus::Add_Sample, 0);
                s_AddSample = AddSample::Send_Add_Sample_Cmd;
            }
        } else if (mAddSample->slides.isEmpty() == false) {
            mHandlingSlide = mAddSample->slides.takeFirst();
        } else {
            qDebug() << "Error" << mAddSample->sid();
            qFatal("WaitF_NewSlide_Ready Error.");
        }
        break;

        /* 2. Add Sample */
    case AddSample::Send_Add_Sample_Cmd:
        dev->sample()->cmd_AddSample_Normal(mAddSample->addVolume());
        s_AddSample = AddSample::WaitF_Add_Sample_Done;
        break;
    case AddSample::WaitF_Add_Sample_Done:
        if (dev->sample()->isFuncDone("AddSample_Normal")) {
            mHandlingSlide->setStatus(SmearStatus::Add_Sample, 1);
            RtSampleManager::GetInstance()->sendUISlideStatus(mHandlingSlide, SlideProcessState::Sampling);
            mHandlingSlide = nullptr;

            if (mAddSample->slides.isEmpty() == false) {
                qDebug() << "slideslist is not finish:" << mAddSample->sid() << mAddSample->slides;
                s_AddSample = AddSample::WaitF_NewSlide_Ready;
            } else {
                mAddSample->setStatus(SampleStatus::Add_Sample, 1);
                logProcess("Add Sample", 2, "Slides add sample finished.", mAddSample->sid());
                s_AddSample = AddSample::Clean_Needles;
            }  
        }
        break;

        /* 3. Clean Needle */
    case AddSample::Clean_Needles:
        logProcess("Add Sample", 3, "CleanNeedle", mAddSample->sid());
        if (mAddSample->isRet()) {
            if (mAddSample->getStatus(SampleStatus::Ret_Hatch) == 1) {
                m_cleanApi = "Clean_AddNeedle";
                dev->sample()->cmd_Clean_AddNeedle();
            } else {
                m_cleanApi = "Clean_AddNeedle_Maintain";
                dev->sample()->cmd_Clean_AddNeedle_Maintain();
            }
        } else {
            m_cleanApi = "Clean_AddNeedle";
            dev->sample()->cmd_Clean_AddNeedle();
        }
        s_AddSample = AddSample::WaitF_Clean_Needles_Done;
        break;
    case AddSample::WaitF_Clean_Needles_Done:        
        if (dev->sample()->isFuncDone(m_cleanApi)) {
            m_isNeedleClean = true;
            s_AddSample = AddSample::Finish;
        }
        break;

    case AddSample::Finish:
        logProcess("AddSample", 99, "Finish", mAddSample->sid());
        mAddSample = nullptr;
        s_AddSample = AddSample::Idle;
        break;

    default:
        break;
    }
}

void MSampling::ReturnTubeFrMixProcess()
{
    switch (s_Recycle) {        
    case RecycleTupe::Idle:
        if (mRecycleSample != nullptr) {
            if (mRecycleSample->getStatus(SampleStatus::Test_Finished) == 1) {
                if (mRecycleSample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
                    if (m_isRecycleFinished == false) {
                        logProcess("Recycle", 1, "Start", mRecycleSample->sid());
                        s_Recycle = RecycleTupe::Move_Tube_To_Exit_Cmd;
                    }
                }
            }
        }
        break;

    case RecycleTupe::Move_Tube_To_Exit_Cmd:
        cmd_ReturnSampleTubeToOut(mRecycleSample->isMiniBlood());
        s_Recycle = RecycleTupe::WaitF_Move_To_Exit_Done;
        break;
    case RecycleTupe::WaitF_Move_To_Exit_Done:
        if (dev->sample()->isFuncDone(m_recycle_api)) {

            /* record */
            recordModuleSampleState("cart", true);

            if (m_isRecycleFinished == false) {
                s_Recycle = RecycleTupe::Send_Recycle_Cmd;
            }
        }
        break;

    case RecycleTupe::Send_Recycle_Cmd:
    {
        bool ok = cmd_ReturnTubeFrCart(mRecycleSample->isEmergency());
        if (ok) {
            mRecycleSample->setStatus(SampleStatus::Recycle_To_Rack, 0);

            /* record */
            recordModuleSampleState("gripper", "doing_sample", mRecycleSample);

            s_Recycle = RecycleTupe::WaitF_Recycle_Done;
        }
    }
        break;
    case RecycleTupe::WaitF_Recycle_Done:
        if (dev->sample()->isFuncDone("ReturnTubeFrCart")) {

            /* record */
            recordModuleSampleState("gripper", "doing_sample", nullptr);
            recordModuleSampleState("cart", "doing_sample", nullptr);

            mRecycleSample->setStatus(SampleStatus::Recycle_To_Rack, 1);
            logProcess("Recycle", 99, "Finish", mRecycleSample->sid());
            m_isRecycleFinished = true;
            s_Recycle = RecycleTupe::Finish;
        }
        break;

    case RecycleTupe::Finish:
        if (s_Sampling == Sampling::Finish) {
            if (mAddSample == nullptr || mAddSample->slides.count() < 2)
            {
                if (m_isUnited) {
                    RtSampleManager::GetInstance()->removeSampleOne(mRecycleSample->sid());
                }
                mRecycleSample = nullptr;
                mSamplingSample = nullptr;
                s_Sampling = Sampling::Idle;
                s_Recycle = RecycleTupe::Idle;
            } else if (mRecycleSample->isRet()
                      /* || mRecycleSample->getStatus(SampleStatus::Add_Sample) == 1*/) {//todo
                if (m_isUnited) {
                    RtSampleManager::GetInstance()->removeSampleOne(mRecycleSample->sid());
                }
                mRecycleSample = nullptr;
                mSamplingSample = nullptr;
                s_Sampling = Sampling::Idle;
                s_Recycle = RecycleTupe::Idle;
            }
        }
        break;
    }
}

void MSampling::startHatch()
{
    if (m_hatchFinished == true) {
        m_hatchFinished = false;
        mHatchTimer->start();
        qDebug() << mRetSample->sid() << "Start Hatching...";
    }
}

void MSampling::hatchRetProcess()
{
    switch (s_ret) {
    case RetState::Idle:
        if (mRetSample != nullptr) {
            s_ret = RetState::Start_Hatch;
        }
        break;

    case RetState::Start_Hatch:
        mRetSample->setStatus(SampleStatus::Ret_Hatch, 0);
        startHatch();
        s_ret = RetState::WaitF_Hatch_Done;
        break;
    case RetState::WaitF_Hatch_Done:
        if (m_hatchFinished == true) {
            qDebug() << mRetSample->sid() << "Hatching finished.";
            mRetSample->setStatus(SampleStatus::Ret_Hatch, 1);
            s_ret = RetState::Lock_Other_Process;
        }
        break;

    case RetState::Lock_Other_Process:
        m_ret_lock = true;
        s_ret = RetState::WaitF_Lock_Other_Process_Done;
        break;
    case RetState::WaitF_Lock_Other_Process_Done:
        if (mSamplingSample == nullptr
                && s_Sampling == Sampling::Idle
                && mUnsamplingList.isEmpty()) {
            if (mAddSample == nullptr
                    && s_AddSample == AddSample::Idle) {
                s_ret = RetState::Take_Ret_Sample;
            }
        }
        break;

    case RetState::Take_Ret_Sample:
        m_isNeedleClean = false;
        dev->sample()->cmd_Take_Mix_Ret(50);
        s_ret = RetState::WaitF_Take_Ret_Sample_Done;
        break;
    case RetState::WaitF_Take_Ret_Sample_Done:
        if (dev->sample()->isFuncDone("Take_Mix_Ret")) {
            s_ret = RetState::Add_Sample;
        }
        break;

    case RetState::Add_Sample:
        mSlideBoxMgr->addNewSlideRequest(mRetSample->slides.first());
        mSmear->addNewSlideRequest(mRetSample->slides.first());
        mAddSample = mRetSample;

        s_AddSample = AddSample::WaitF_NewSlide_Ready;
        s_ret = RetState::WaitF_Add_Sample_Done;
        break;
    case RetState::WaitF_Add_Sample_Done:
        if (mRetSample->getStatus(SampleStatus::Add_Sample) == 1) {
            s_ret = RetState::Finish;
        }
        break;

    case RetState::Finish:
        m_handle_ret = false;
        m_ret_lock = false;
        mRetSample = nullptr;
        s_ret = RetState::Idle;
        break;
    }
}

void MSampling::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api == "NeedleStab_TakeSample" || api == "NeedleAdd_TakeSample") {
        if (mSamplingSample != nullptr) {
            if (resValue.isObject()) {
                QJsonObject obj = resValue.toObject();
                QJsonArray arr = obj.value("value_16").toArray();
                if (!arr.isEmpty()) {
                    int viscosity = arr.first().toInt();
                    mSamplingSample->setViscosity(viscosity);
                }
            }
            mSamplingSample->getSmearParams();
        }
    }

#if 0
                    bool ok = m_viscosity->setSmearParams(viscosity, mHandlingSample);
                    qDebug() << "get getSmearParams:" << ok;
#endif
}

