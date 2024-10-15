#include "module_manager.h"
#include "process/process_manager.h"
#include "track/track_manager.h"
#include "smear/smear_manager.h"
#include "stain/stain_manager.h"
#include "light/m_lights.h"
#include "unity/m_unity_task.h"
#include "device/rt_device_manager.h"
#include "sql/f_sql_database_manager.h"
#include "rack/rt_rack_manager.h"
#include "f_common.h"

DModuleManager *DModuleManager::GetInstance()
{
    static DModuleManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new DModuleManager();
    }
    return instance;
}

DModuleManager::DModuleManager(QObject *parent) : QObject(parent)
{
    qDebug("[DModuleManager] init..");
    m_isUnited = FCommon::GetInstance()->isUnited();

    _init();
    _logDbInit();
    qDebug("[DModuleManager] init..OK.");
    qDebug() << " ";
}

void DModuleManager::_init()
{
    m_isRuning = false;

    mUnityTask = new MUnityTask(this);
    mUnityTask->setManager(this);
    mUnityTask->setDeviceManager(RtDeviceManager::GetInstance());

    auto process = MProcessManager::GetInstance();
    connect(process->mBootProc, &MProcessBase::onProcessFinished_signal, this, [=](const QString &process){
        if (process == "boot") {
            start();
        }
    });

    auto track = TrackManager::GetInstance();
    auto smear = SmearManager::GetInstance();
    auto stain = StainManager::GetInstance();

    mUnityTask->setSampling(smear->mSampling);
    track->mCarts->mCart1->setSampling(smear->mSampling);
    track->mCarts->mCart2->setSampling(smear->mSampling);
    track->mEmergency->setSampling(smear->mSampling);
    smear->mSmear->setStainCart(stain->mStainCart);
    process->mRemoveRemianTube->setEmergency(track->mEmergency);

    auto light = MLights::GetInstance();
    light->setDeviceManager(RtDeviceManager::GetInstance());
}

void DModuleManager::_logDbInit()
{
    auto logDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (logDb) {
        logDb->deleteRecord("track");
        logDb->deleteRecord("smear");
        logDb->deleteRecord("stain");

        bool clear = FCommon::GetInstance()->getConfigValue("log", "clear_sample_history").toBool();
        if (clear) {
            logDb->deleteRecord("samples", QJsonObject(), false);
            logDb->deleteRecord("slides", QJsonObject(), false);
        }
    }
}

void DModuleManager::startAutoTest()
{
    if (m_isRuning == false) {
        m_isRuning = true;

        if (m_isUnited == false) {
            TrackManager::GetInstance()->mImport->startReceieNewRack();
            TrackManager::GetInstance()->mCarts->start();
        }
        qInfo() << ">>> [AutoTest] Start. <<<  RUNNING:" << m_isRuning;
    }
}

void DModuleManager::suspendAutoTest()
{
    if (m_isRuning == true) {
        m_isRuning = false;

        if (m_isUnited == false) {
            TrackManager::GetInstance()->mCarts->suspend();
        }
        qInfo() << ">>> [AutoTest] Suspend. <<<  RUNNING:" << m_isRuning;
    }
}

void DModuleManager::stopAutoTest()
{
    if (m_isRuning == true) {
        m_isRuning = false;

        if (m_isUnited == false) {
            TrackManager::GetInstance()->stopDetectNewRack();
        }
        qInfo() << ">>> [AutoTest] Stopped. <<<  RUNNING:" << m_isRuning;
    }
}

void DModuleManager::abortAutoTest()
{
    if (m_isRuning == true) {
        m_isRuning = false;

        if (m_isUnited == false) {
            TrackManager::GetInstance()->abort();
        }
        qInfo() << ">>> [AutoTest] Aborted. <<<  RUNNING:" << m_isRuning;
    }
}

void DModuleManager::start()
{
    if (m_isUnited == false) {
        TrackManager::GetInstance()->start();
    }
    StainManager::GetInstance()->start();
    SmearManager::GetInstance()->start();
    MLights::GetInstance()->start();
}

void DModuleManager::startOne(const QString &mid)
{
    if (mid.contains("track")) {
        TrackManager::GetInstance()->start();
    } else if (mid.contains("smear") || mid.contains("sample")) {
        SmearManager::GetInstance()->start();
    } else if (mid.contains("stain")) {
        StainManager::GetInstance()->start();
    }
}

void DModuleManager::reset()
{
    if (m_isUnited == false) {
        TrackManager::GetInstance()->reset();
    }
    StainManager::GetInstance()->reset();
    SmearManager::GetInstance()->reset();
    MLights::GetInstance()->reset();

    auto logDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (logDb) {
        logDb->deleteRecord("track");
        logDb->deleteRecord("smear");
        logDb->deleteRecord("stain");
    }
}

void DModuleManager::reset(const QString &mid)
{
    if (mid.contains("track")) {
        TrackManager::GetInstance()->reset();
    } else if (mid.contains("smear") || mid.contains("sample")) {
        SmearManager::GetInstance()->reset();
    } else if (mid.contains("stain")) {
        StainManager::GetInstance()->reset();
    } else {
        reset();
    }
}

void DModuleManager::stop()
{
    TrackManager::GetInstance()->stop();
    StainManager::GetInstance()->stop();
    SmearManager::GetInstance()->stop();
    MLights::GetInstance()->stop();
}

void DModuleManager::stop(const QString &mid)
{
    if (mid.contains("track")) {
        TrackManager::GetInstance()->stop();
    } else if (mid.contains("smear") || mid.contains("sample")) {
        SmearManager::GetInstance()->stop();
    } else if (mid.contains("stain")) {
        StainManager::GetInstance()->stop();
    } else {
        stop();
    }
}

