#include "process_manager.h"
#include "module/track/track_manager.h"

MProcessManager *MProcessManager::GetInstance()
{
    static MProcessManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new MProcessManager();
    }
    return instance;
}

MProcessManager::MProcessManager(QObject *parent)
    : MManagerBase{"process", parent}
{
    mBootProc = new MBootProcess(this);
    mResetProc = new MResetProcess(this);
    mCleanLiquidProc = new MCleanLiquidSystemProcess(this);
    mCleanAllSlotsProc = new MCleanAllSlotsProcess(this);
    mShutDownProc = new MShutDownProcess(this);
    mDrainFilterProc = new MDrainFilterProcess(this);
    mConfigTask = new MConfigTask(this);
    mRemoveRemianTube = new MRemoveRemainTube(this);
    mStainAgingProc = new MStainAgingProcess(this);
    mSmearAgingProc = new MSmearAgingProcess(this);
    mRemoveRemainSlides = new MRemoveRemainStainSlides(this);
    mPerfuseProc = new MPerfuseProcess(this);
    mSleepProc = new MSleepProcess(this);
    mCheckSensorsProc = new MCheckSensorsProcess(this);
    mNeedleMaintianProc = new MNeedleMaintainProcess(this);

    addModule(mBootProc, mResetProc);
    addModule(mCleanLiquidProc, mCleanAllSlotsProc);
    addModule(mShutDownProc, mDrainFilterProc);
    addModule(mConfigTask);
    addModule(mRemoveRemianTube, mRemoveRemainSlides);
    addModule(mStainAgingProc, mSmearAgingProc);
    addModule(mPerfuseProc);
    addModule(mSleepProc);
    addModule(mCheckSensorsProc);
    addModule(mNeedleMaintianProc);

    mProcessMap.insert("boot", mBootProc);
    mProcessMap.insert("reset", mResetProc);
    mProcessMap.insert("cleanLiquid", mCleanLiquidProc);
    mProcessMap.insert("cleanAllSlots", mCleanAllSlotsProc);
    mProcessMap.insert("shutDown", mShutDownProc);
    mProcessMap.insert("removeRemianTube", mRemoveRemianTube);
    mProcessMap.insert("removeRemainStainSlides", mRemoveRemainSlides);
    mProcessMap.insert("stainAging", mStainAgingProc);
    mProcessMap.insert("smearAging", mSmearAgingProc);
    mProcessMap.insert("perfuse", mPerfuseProc);
    mProcessMap.insert("sleep", mSleepProc);
    mProcessMap.insert("checkSensors", mCheckSensorsProc);
    mProcessMap.insert("needleMaintain", mNeedleMaintianProc);

    mRemoveRemianTube->setEmergency(TrackManager::GetInstance()->mEmergency);
}

void MProcessManager::startProcess(const QString &process, quint64 id)
{
    if (mProcessMap.contains(process)) {
        auto pro = mProcessMap.value(process);
        pro->startProcess(id);
    }
}

void MProcessManager::startProcess(const JPacket &packet)
{
    auto obj = packet.paramsValue.toObject();
    if (obj.contains("process")) {
        auto process = obj.value("process").toString();
        startProcess(process, packet.id);
    }
}

void MProcessManager::stopProcess(const QString &process, quint64 id)
{
    if (mProcessMap.contains(process)) {
        auto pro = mProcessMap.value(process);
        pro->stopProcess(id);
    } else {
        qDebug() << "can NOT handle stopProcess:" << process << id;
    }
}

void MProcessManager::stopProcess(const JPacket &packet)
{
    auto obj = packet.paramsValue.toObject();
    if (obj.contains("process")) {
        auto process = obj.value("process").toString();
        stopProcess(process, packet.id);
    }
}

void MProcessManager::resetProcess(const QString &process, quint64 id)
{
    if (mProcessMap.contains(process)) {
        auto pro = mProcessMap.value(process);
        pro->resetProcess(id);
    } else {
        qDebug() << "can NOT handle stopProcess:" << process << id;
    }
}

void MProcessManager::resetProcess(const JPacket &packet)
{
    auto obj = packet.paramsValue.toObject();
    if (obj.contains("process")) {
        auto process = obj.value("process").toString();
        resetProcess(process, packet.id);
    }
}

void MProcessManager::handleProcess(const JPacket &packet)
{
    if (packet.module == "Process") {
        if (packet.api == "Start") {
            startProcess(packet);
        } else if (packet.api == "Stop") {
            stopProcess(packet);
        } else if (packet.api == "Reset") {
            resetProcess(packet);
        }
    }
}

