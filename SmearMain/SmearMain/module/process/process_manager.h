#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "m_manager_base.h"
#include "m_process_base.h"

#include "m_boot_process.h"
#include "m_clean_liquid_system_process.h"
#include "m_reset_process.h"
#include "m_clean_all_slots_process.h"
#include "m_shutdown_process.h"
#include "m_drain_filter_process.h"
#include "m_remove_remain_tube.h"
#include "m_config_task.h"
#include "m_stain_aging_process.h"
#include "m_smear_aging_process.h"
#include "m_perfuse_process.h"
#include "m_remove_remain_stain_slides.h"
#include "m_sleep_process.h"
#include "m_check_sensors_process.h"
#include "m_needle_maintian_process.h"

class MProcessManager : public MManagerBase
{
    Q_OBJECT
public:
    static MProcessManager *GetInstance();

    void startProcess(const QString &process, quint64 id = 0);
    void startProcess(const JPacket &packet);
    void stopProcess(const QString &process, quint64 id = 0);
    void stopProcess(const JPacket &packet);
    void resetProcess(const QString &process, quint64 id = 0);
    void resetProcess(const JPacket &packet);

    void handleProcess(const JPacket &packet);

    MBootProcess *mBootProc;
    MResetProcess *mResetProc;
    MCleanLiquidSystemProcess *mCleanLiquidProc;
    MCleanAllSlotsProcess *mCleanAllSlotsProc;
    MShutDownProcess *mShutDownProc;
    MDrainFilterProcess *mDrainFilterProc;
    MConfigTask *mConfigTask;
    MRemoveRemainTube *mRemoveRemianTube;
    MStainAgingProcess *mStainAgingProc;
    MSmearAgingProcess *mSmearAgingProc;
    MRemoveRemainStainSlides *mRemoveRemainSlides;
    MPerfuseProcess *mPerfuseProc;
    MSleepProcess *mSleepProc;
    MCheckSensorsProcess *mCheckSensorsProc;
    MNeedleMaintainProcess *mNeedleMaintianProc;

private:
    explicit MProcessManager(QObject *parent = nullptr);
    QMap<QString, MProcessBase*> mProcessMap;
};

#endif // PROCESS_MANAGER_H
