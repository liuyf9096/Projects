#include "m_sampling2.h"
#include "m_slidestore.h"
#include "m_smear.h"
#include "record/rt_machine_record.h"

#define Ret

MSampling::MSampling(const QString &mid, QObject *parent)
    : DModuleBase(mid, "sampling", parent)
{
    m_SamplingVolume = 240;
    m_AddSampleVolume = 3;

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

    mHandlingSlide = nullptr;
    mHandlingSample = nullptr;
    mAddSample = nullptr;
    mRetSample = nullptr;

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

bool MSampling::isReceivable(QSharedPointer<RtSample> sample)
{
    if (m_ret_lock == true) {
        return false;
    }
    if (sample != nullptr) {
        if (sample->isRet()) {
            if (m_handle_ret == true) {
                return false;
            }
        }
    }
    if (mHandlingSample == nullptr && mUnsamplingList.isEmpty()) {
        return true;
    }
    return false;
}

bool MSampling::receiveNewSample(QSharedPointer<RtSample> sample)
{
    if (sample == nullptr) {
        return false;
    }
    if (isReceivable(sample)) {
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

bool MSampling::receiveEmergencySample(QSharedPointer<RtSample> sample)
{
    if (m_ret_lock == true) {
        return false;
    }
    if (sample->isEmergency()) {
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
    if (mHandlingSample != nullptr && mHandlingSample->sid() == sample->sid()) {
        if (mHandlingSample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
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

bool MSampling::cmd_MixNewSampleTube(QSharedPointer<RtSample> sample)
{
    if (sample->isMiniBlood()) {
        m_api = "TubeToNormalWithoutMix";
        return dev->sample()->cmd_TubeToNormalWithoutMix();
#if 0
        if (sample->isEmergency()) {
            m_api = "TubeToNormalWithoutMix";
            return dev->sample()->cmd_TubeToNormalWithoutMix();
        } else {
            m_api = "TubeToMini";
            return dev->sample()->cmd_TubeToMini();
        }
#endif
    } else {    // WholeBlood
        if (sample->isEmergency() && false) {
            m_api = "TubeToNormalWithoutMix";
            return dev->sample()->cmd_TubeToNormalWithoutMix();
        } else {
            m_api = "TubeToNormal";
            return dev->sample()->cmd_TubeToNormal();
        }
    }
    return false;
}

bool MSampling::cmd_NeedleTakeSample(QSharedPointer<RtSample> sample, int volume)
{
    if (sample->isMiniBlood()) {
        m_api = "NeedleAdd_TakeSample";
        return dev->sample()->cmd_NeedleAdd_TakeSample(volume);
    } else {    // WholeBlood
        if (sample->isEmergency() && false) {
            m_api = "NeedleAdd_TakeSample";
            return dev->sample()->cmd_NeedleAdd_TakeSample(volume);
        } else {
            m_api = "NeedleStab_TakeSample";
            return dev->sample()->cmd_NeedleStab_TakeSample(volume);
        }
    }
    return false;
}

bool MSampling::cmd_ReturnSampleTube(bool isEmergency, bool isMiniBlood)
{
    SamplePos pos0, pos1;

    if (isMiniBlood == true) {
        pos0 = Pos_Normal;      /* Pos_Mini is obsolete */
    } else {
        pos0 = Pos_Normal;
    }
    if (isEmergency == true) {
        pos1 = Pos_Emergency;
    } else {
        pos1 = Pos_Rack;
    }
    return dev->sample()->cmd_ReturnSampleTube(pos0, pos1);
}

void MSampling::onStateTimer_slot()
{
    samplingProcess();
    addSampleProcess();
    recycleTubeProcess();
    hatchRetProcess();
}

void MSampling::samplingProcess()
{
    if (dev->sample()->isResetOk() == false) {
        return;
    }

    switch (s_Sampling) {
    case Sampling::Idle:
        if (mHandlingSample == nullptr && mUnsamplingList.isEmpty() == false) {
            mHandlingSample = mUnsamplingList.takeFirst();
            mHandlingSample->setStatus(SampleStatus::Test_Finished, 0);

            qDebug() << "slide list:" << mHandlingSample->slides;
            s_Sampling = Sampling::Receive_New_Sample;
        }
        break;

    case Sampling::Receive_New_Sample:
        if (mHandlingSample->slides.isEmpty() == false) {
            if (mHandlingSample->isRet() == false) {
                for (int i = 0; i < mHandlingSample->slides.count(); ++i) {
                    mSlideBoxMgr->addNewSlideRequest(mHandlingSample->slides.at(i));
                    mSmear->addNewSlideRequest(mHandlingSample->slides.at(i));
                }
            }

            emit onTakeNewSample_signal(mHandlingSample->sid());
            logProcess("Sampling", 1, "Receive Sample Tube Request", mHandlingSample->sid());
            s_Sampling = Sampling::Get_New_SampleTube;
        } else {
            qCritical() << "slides.isEmpty";
        }
        break;

        /* 1. Get New Tube */
    case Sampling::Get_New_SampleTube:        
        cmd_GetNewSampleTube(mHandlingSample->isEmergency());
        logProcess("Sampling", 2, "Get New Tube", mHandlingSample->sid());

    {
        /* record */
        auto record = RtMachineRecord::GetInstance();
        record->currentSample = mHandlingSample->sid();
        record->isGrip = true;
        record->isEmergency = mHandlingSample->isEmergency();
        record->rack_id = mHandlingSample->rack_id();
        record->rack_pos = mHandlingSample->rack_pos();
        record->write();
    }

        s_Sampling = Sampling::WaitF_Get_New_SampleTube_Done;
        break;
    case Sampling::WaitF_Get_New_SampleTube_Done:
        if (dev->sample()->isFuncDone("GetNewTube")) {
            /* record */
            auto record = RtMachineRecord::GetInstance();
            record->isGrip = false;
            record->write();

            mHandlingSample->setStatus(SampleStatus::Send_To_Test, 1);
            m_isReceiveFinished = true;
            s_Sampling = Sampling::Send_New_SampleTube;
        }
        break;

    case Sampling::Send_New_SampleTube:
        cmd_MixNewSampleTube(mHandlingSample);
        mHandlingSample->setStatus(SampleStatus::Mixing, 0);
        s_Sampling = Sampling::WaitF_Mix_New_SampleTube_Done;
        break;
    case Sampling::WaitF_Mix_New_SampleTube_Done:
        if (dev->sample()->isFuncDone(m_api)) {
            mHandlingSample->setStatus(SampleStatus::Mixing, 1);
            s_Sampling = Sampling::Sampling;
        }
        break;

        /* 2. Sampling */
    case Sampling::Sampling:
        if (m_isNeedleClean == true) {
            m_isNeedleClean = false;

            cmd_NeedleTakeSample(mHandlingSample, m_SamplingVolume);
            mHandlingSample->setStatus(SampleStatus::Sampling, 0);
            RtSampleManager::GetInstance()->sendUISampleStatus(mHandlingSample, Process_Sampling);

#ifdef Ret
            if (mHandlingSample->isRet()) {
                dev->sample()->cmd_PoolRet_Fill();
            }
#endif
            s_Sampling = Sampling::WaitF_Sampling_Done;
        }
        break;
    case Sampling::WaitF_Sampling_Done:
        if (dev->sample()->isFuncDone(m_api)) {
            mHandlingSample->setStatus(SampleStatus::Sampling, 1);
            mHandlingSample->setStatus(SampleStatus::Test_Finished, 1);

            s_Sampling = Sampling::Send_To_Add_Sample;
        }
        break;

    case Sampling::Send_To_Add_Sample:
        if (mAddSample == nullptr) {
            mAddSample = mHandlingSample;

            logProcess("Sampling", 99, "Finish", mHandlingSample->sid());
            s_Sampling = Sampling::Finish;
        }
        break;

    case Sampling::Finish:
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
                    mAddSample->setStatus(SampleStatus::Prepare_Blood, 0);
                    logProcess("AddSample", 1, "Start", mAddSample->sid());
                    s_AddSample = AddSample::Prepare_Sampling;
                }
            }
        }
        break;

        /* 1. Prepare Sampling */
    case AddSample::Prepare_Sampling:
        if (mAddSample->isMiniBlood()) {
            mAddSample->setStatus(SampleStatus::Prepare_Blood, 1);
            s_AddSample = AddSample::WaitF_NewSlide_Ready;
        } else {
            dev->sample()->cmd_PrepareSampling();
            s_AddSample = AddSample::WaitF_Prepare_Sampling_Done;
        }
        break;
    case AddSample::WaitF_Prepare_Sampling_Done:
        if (dev->sample()->isFuncDone("PrepareSampling")) {
            mAddSample->setStatus(SampleStatus::Prepare_Blood, 1);

            s_AddSample = AddSample::WaitF_NewSlide_Ready;

#ifdef Ret
            if (mAddSample->isRet()) {
                s_AddSample = AddSample::Ret_Mix_Sample;
            } else {
                s_AddSample = AddSample::WaitF_NewSlide_Ready;
            }
#endif
        }
        break;

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

        /* 2. Wait for New Slide Ready */
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

        /* 3. Add Sample */
    case AddSample::Send_Add_Sample_Cmd:
        dev->sample()->cmd_AddSample_Normal(mAddSample->addVolume());
        s_AddSample = AddSample::WaitF_Add_Sample_Done;
        break;
    case AddSample::WaitF_Add_Sample_Done:
        if (dev->sample()->isFuncDone("AddSample_Normal")) {
            mHandlingSlide->setStatus(SmearStatus::Add_Sample, 1);
            RtSampleManager::GetInstance()->sendUISlideStatus(mHandlingSlide, Process_Sampling);
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

        /* 3. Clean Needles */
    case AddSample::Clean_Needles:
        logProcess("Add Sample", 3, "CleanNeedles", mAddSample->sid());
        if (mAddSample->isRet()) {
            if (mAddSample->getStatus(SampleStatus::Ret_Hatch) == 1) {
                m_cleanApi = "Clean_AddNeedle";
                dev->sample()->cmd_Clean_AddNeedle();
            } else {
                m_cleanApi = "CleanNeedles_Maintain";
                dev->sample()->cmd_CleanNeedles_Maintain();
            }
        } else {
            m_cleanApi = "CleanNeedles";
            dev->sample()->cmd_CleanNeedles();
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
    }
}

void MSampling::recycleTubeProcess()
{
    switch (s_Recycle) {
    case RecycleTupe::Idle:
        if (mHandlingSample != nullptr) {
            if (mHandlingSample->getStatus(SampleStatus::Test_Finished) == 1) {
                if (mHandlingSample->getStatus(SampleStatus::Recycle_To_Rack) == 0) {
                    if (m_isRecycleFinished == false) {
                        logProcess("Recycle", 1, "Start", mHandlingSample->sid());
                        s_Recycle = RecycleTupe::Send_Recycle_Cmd;
                    }
                }
            }
        }
        break;

    case RecycleTupe::Send_Recycle_Cmd:
        mHandlingSample->setStatus(SampleStatus::Recycle_To_Rack, 0);
        cmd_ReturnSampleTube(mHandlingSample->isEmergency(), mHandlingSample->isMiniBlood());

    {
        /* record */
        auto record = RtMachineRecord::GetInstance();
        record->isGrip = true;
        record->write();
    }

        s_Recycle = RecycleTupe::WaitF_Recycle_Done;
        break;
    case RecycleTupe::WaitF_Recycle_Done:
        if (dev->sample()->isFuncDone("ReturnSampleTube")) {

            /* record */
            auto record = RtMachineRecord::GetInstance();
            record->clearSampling();
            record->write();

            mHandlingSample->setStatus(SampleStatus::Recycle_To_Rack, 1);
            logProcess("Recycle", 99, "Finish", mHandlingSample->sid());
            m_isRecycleFinished = true;
            s_Recycle = RecycleTupe::Finish;
        }
        break;

    case RecycleTupe::Finish:
        if (s_Sampling == Sampling::Finish) {
            if (mAddSample->slides.count() < 2) {
                mHandlingSample = nullptr;
                s_Sampling = Sampling::Idle;
                s_Recycle = RecycleTupe::Idle;
            } else if (mHandlingSample->isRet()
                      /* || mHandlingSample->getStatus(SampleStatus::Add_Sample) == 1*/) {//todo
                mHandlingSample = nullptr;
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
        if (mHandlingSample == nullptr && s_Sampling == Sampling::Idle && mUnsamplingList.isEmpty()) {
            if (mAddSample == nullptr && s_AddSample == AddSample::Idle) {
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
        if (mHandlingSample != nullptr) {
            if (resValue.isObject()) {
                QJsonObject obj = resValue.toObject();
                QJsonArray arr = obj.value("value_16").toArray();
                if (!arr.isEmpty()) {
                    int viscosity = arr.first().toInt();
                    mHandlingSample->setViscosity(viscosity);
                }
            }
            mHandlingSample->getSmearParams();
        }
    }

#if 0
                    bool ok = m_viscosity->setSmearParams(viscosity, mHandlingSample);
                    qDebug() << "get getSmearParams:" << ok;
#endif
}

