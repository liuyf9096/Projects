#include "slots_manager.h"
#include "f_common.h"
#include "stain/recyclebox/m_recyclebox_mgr.h"
#include "messagecenter/f_message_center.h"
#include "stain/gripper/gripper_manager.h"
#include "sql/f_sql_database_manager.h"
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

SlotsManager *SlotsManager::GetInstance()
{
    static SlotsManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new SlotsManager();
    }
    return instance;
}

SlotsManager::SlotsManager(QObject *parent)
    : MManagerBase{"slots", parent}
{
    /* slot groups */
    mGroupFix = new SlotGroupFix("fix", this);
    mGroupFixDry = new SlotGroupFixDry("fixdry", this);
    mGroupC1 = new SlotGroupC1("c1", this);
    mGroupC2 = new SlotGroupC2("c2", this);
    mGroupWash = new SlotGroupWash("wash", this);
    mGroupTransfer = new SlotGroupTransfer("transfer", this);

    /* create slots */
    _setSlots();

    int cycleTimeSec = FCommon::GetInstance()->getConfigValue("stain", "slot_duration_cycle").toInt();
    if (cycleTimeSec == 0) {
        cycleTimeSec = 45; // default
    }
    _setSlotDefaultDuration(cycleTimeSec);
    qInfo().noquote() << QString("stain period & Check interval:%1s").arg(cycleTimeSec);

    m_drainSlots_id = 0;
    m_cleanSlots_id = 0;
    m_isDrainSlotsFinished = true;
    m_isCleanSlotsFinished = true;
    m_cleanAllSlots = false;
    m_drainWash = false;

    s_drainSlots = DrainSlotsState::Idle;
    s_cleanSlots = CleanSlotsState::Idle;

    m_drainSlotsTimer = new QTimer(this);
    m_drainSlotsTimer->setInterval(100);
    connect(m_drainSlotsTimer, &QTimer::timeout,
            this, &SlotsManager::drainAllSlots_slot);

    m_cleanSlotsTimer = new QTimer(this);
    m_cleanSlotsTimer->setInterval(100);
    connect(m_cleanSlotsTimer, &QTimer::timeout,
            this, &SlotsManager::cleanAllSlots_slot);
}

void SlotsManager::_setSlots()
{
    QJsonObject slotsObj = FCommon::GetInstance()->getConfigValue("slot_setup", "standard", "stain_slot").toObject();
    if (slotsObj.isEmpty()) {
        qFatal("Missing slot config file");
    }
    if (slotsObj.contains("fix")) {
        QJsonObject obj = slotsObj.value("fix").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupFix->setSlot(from, to);
    }
    if (slotsObj.contains("fixdry")) {
        QJsonObject obj = slotsObj.value("fixdry").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupFixDry->setSlot(from, to);
    }
    if (slotsObj.contains("c1")) {
        QJsonObject obj = slotsObj.value("c1").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupC1->setSlot(from, to);
    }
    if (slotsObj.contains("c2")) {
        QJsonObject obj = slotsObj.value("c2").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupC2->setSlot(from, to);
    }
    if (slotsObj.contains("wash")) {
        QJsonObject obj = slotsObj.value("wash").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupWash->setSlot(from, to);
    }
    if (slotsObj.contains("transfer")) {
        QJsonObject obj = slotsObj.value("transfer").toObject();
        int from = obj.value("from").toInt();
        int to = obj.value("to").toInt();
        mGroupTransfer->setSlot(from, to);
    }

    mSlotGroupMap.insert("fix",      mGroupFix);
    mSlotGroupMap.insert("fixdry",   mGroupFixDry);
    mSlotGroupMap.insert("c1",       mGroupC1);
    mSlotGroupMap.insert("c2",       mGroupC2);
    mSlotGroupMap.insert("wash",     mGroupWash);
    mSlotGroupMap.insert("transfer", mGroupTransfer);

    foreach (auto group, mSlotGroupMap) {
        connect(group, &SlotGroupBase::onGripperRequest_signal,
                GripperManager::GetInstance(), &GripperManager::onGripperRequest_slot);
    }
}

void SlotsManager::_setSlotDefaultDuration(int cycle_sec)
{
#if 1
    mGroupFix->setDuration(     1 * cycle_sec);
    mGroupFixDry->setDuration(  1 * cycle_sec);
    mGroupC1->setDuration(      1 * cycle_sec);
    mGroupC2->setDuration(      1 * cycle_sec);
    mGroupWash->setDuration(    1 * cycle_sec);
#else
    mGroupFix->setDuration(     1 * cycle_sec);
    mGroupFixDry->setDuration(  1 * cycle_sec);
    mGroupC1->setDuration(      3 * cycle_sec);
    mGroupC2->setDuration(      8 * cycle_sec);
    mGroupWash->setDuration(    1 * cycle_sec);
#endif
}

void SlotsManager::setSlotParams(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        if (obj.contains("group")) {
            QString groupName = obj.value("group").toString();
            if (mSlotGroupMap.contains(groupName))
            {
                auto group = mSlotGroupMap.value(groupName);

                if (obj.contains("duration")) {
                    int duration = obj.value("duration").toInt();
                    group->setDuration(duration);
                }

                if (groupName == "wash") {
                    int count = obj.value("count").toInt();
                    mGroupWash->setWashCount(count);
                } else if (groupName.startsWith("c", Qt::CaseInsensitive)) {
                    if (obj.contains("ratio")) {
                        QJsonValue ratioValue = obj.value("ratio");
                        if (ratioValue.isObject()) {
                            QJsonObject ratioObj = ratioValue.toObject();
                            int a = ratioObj.value("a").toInt();
                            int b = ratioObj.value("b").toInt();
                            if (groupName == "c1") {
                                mGroupC1->setSolutionRate(a, b);
                            } else if (groupName == "c2") {
                                mGroupC2->setSolutionRate(a, b);
                            }
                        }
                    }
                }
            } else {
                qWarning() << "Can NOT find the slot group:" << groupName;
            }
        }
    }
}

void SlotsManager::setStainMethod(const QString &method)
{
    foreach (auto group, mSlotGroupMap) {
        group->setStainMethod(method);
    }

    qInfo() << "Set Stain Method:" << method;
}

bool SlotsManager::containGroup(const QString &group)
{
    return mSlotGroupMap.contains(group);
}

void SlotsManager::drainAllSlots(quint64 id)
{
    if (m_isDrainSlotsFinished == true) {
        m_isDrainSlotsFinished = false;
        m_drainSlotsTimer->start();
        s_drainSlots = DrainSlotsState::Idle;
        if (id > 0) {
            m_drainSlots_id = id;
        }
    }
}

void SlotsManager::drainSlots(QVector<int> arr, quint64 id)
{
    if (arr.count() > 0 && m_isDrainSlotsFinished == true) {
        m_drain_slot_arr = arr;
        m_isDrainSlotsFinished = false;
        m_drainSlotsTimer->start();
        s_drainSlots = DrainSlotsState::Idle;
        if (id > 0) {
            m_drainSlots_id = id;
        }
    }
}

void SlotsManager::cleanAllSlots(JPacket p)
{
    m_cleanAllSlots = true;

    QJsonObject obj = p.paramsValue.toObject();
    if (obj.contains("detergent")) {
        QString detergent = obj.value("detergent").toString();
        if (detergent == "alcohol") {
            cleanAllSlots(Detergent_Alcohol, p.id);
        } else {
            cleanAllSlots(Detergent_Water, p.id);
        }
    } else {
        cleanAllSlots(Detergent_Water, p.id);
    }
}

void SlotsManager::cleanAllSlots(Detergent d, quint64 id)
{
    m_cleanAllSlots = true;

    if (m_isCleanSlotsFinished == true) {
        m_isCleanSlotsFinished = false;
        m_cleanSlotsTimer->start();
        s_cleanSlots = CleanSlotsState::Idle;
        m_detergent = d;
        if (id > 0) {
            m_cleanSlots_id = id;
        }
    }
}

void SlotsManager::cleanSlots(QVector<int> arr, Detergent d, quint64 id)
{
    qDebug() << "clean Slots Start, id:" << id;

    m_cleanAllSlots = false;

    if (arr.count() > 0 && m_isCleanSlotsFinished == true) {
        for (int i = 0; i < arr.count(); ++i) {
            int pos = arr.at(i);
            if (mGroupFix->contains(pos)) {
                m_cleanFixArr.append(pos);
            } else if (mGroupC1->contains(pos)) {
                m_cleanC1Arr.append(pos);
            } else if (mGroupC2->contains(pos)) {
                m_cleanC2Arr.append(pos);
            } else if (mGroupWash->contains(pos)) {
                m_drainWash = true;
            } else {
                qDebug() << "can NOT handle StainSlot_Not_Clean slot pos:" << pos;
            }
        }
        m_isCleanSlotsFinished = false;
        m_cleanSlotsTimer->start();
        s_cleanSlots = CleanSlotsState::Idle;
        m_detergent = d;
        if (id > 0) {
            m_cleanSlots_id = id;
        }
    }
}

void SlotsManager::cleanSlots(QJsonArray arr, Detergent d, quint64 id)
{
    m_cleanAllSlots = false;

    if (arr.count() > 0 && m_isCleanSlotsFinished == true) {
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject obj = arr.at(i).toObject();
            int pos = obj.value("slot_pos").toInt();
            if (mGroupFix->contains(pos)) {
                m_cleanFixArr.append(pos);
            } else if (mGroupC1->contains(pos)) {
                m_cleanC1Arr.append(pos);
            } else if (mGroupC2->contains(pos)) {
                m_cleanC2Arr.append(pos);
            } else if (mGroupWash->contains(pos)) {
                m_drainWash = true;
            } else {
                qDebug() << "can NOT handle StainSlot_Not_Clean slot pos:" << pos;
            }
        }
        m_isCleanSlotsFinished = false;
        m_cleanSlotsTimer->start();
        s_cleanSlots = CleanSlotsState::Idle;
        m_detergent = d;
        if (id > 0) {
            m_cleanSlots_id = id;
        }
    }
}

void SlotsManager::cleanSlots(QJsonArray arr, const QString &detergent, quint64 id)
{
    qDebug() << "cleanSlots:" << arr << detergent << "id:" << id;
    if (detergent == "alcohol") {
        cleanSlots(arr, Detergent_Alcohol, id);
    } else {
        cleanSlots(arr, Detergent_Water, id);
    }
}

void SlotsManager::cleanRemainSlots(quint64 id)
{
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("record");
    if (db) {
        QJsonArray arr = db->selectRecord("slot", "isSolutionFilled=1 OR isDetergentFilled=1");
        if (arr.count() > 0) {
            QVector<int> slotsArr;
            for (int i = 0; i < arr.count(); ++i) {
                QJsonObject obj;
                obj = arr.at(i).toObject();
                int slot_pos = obj.value("slot_pos").toInt();
                slotsArr.append(slot_pos);
            }
            cleanSlots(slotsArr, Detergent_Water, id);
        } else {
            JPacket p(PacketType::Result, id);
            p.resValue == true;
            FMessageCenter::GetInstance()->sendUIMessage(p);
        }
    }
}

void SlotsManager::handleSlotRequest(const JPacket &p)
{
    if (p.api.startsWith("DrainFix")) {
        mGroupFix->handleSlotRequest(p);
    } else if (p.api == "DrainAll") {
        drainAllSlots(p.id);
    } else if (p.api == "CleanAll") {
        cleanAllSlots(p);
    }
}

void SlotsManager::handleRemainSlides(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString group_id = obj.value("group_id").toString();
        int slot_pos = obj.value("slot_pos").toInt();
        QString slide_id = obj.value("slide_id").toString();

        if (mSlotGroupMap.contains(group_id)) {
            auto group = mSlotGroupMap.value(group_id);
            if (group->contains(slot_pos)) {
                group->setRemainSlideSlot(slot_pos, slide_id);
            }
        } else {
            qDebug() << "can NOT handle remain slide:" << obj;
        }
    }
}

void SlotsManager::startFillPool(quint64 id, const QJsonObject &obj)
{
    qDebug() << "todo..." << id << obj;
}

void SlotsManager::takingOutSample(const QString &group_id, const QString &sid, int pos)
{
    qDebug() << __FUNCTION__ << group_id << sid << pos;
}

void SlotsManager::takeOutSample(const QString &group_id, const QString &sid, int pos)
{
    if (mSlotGroupMap.contains(group_id)) {
        auto group = mSlotGroupMap.value(group_id);
        group->takeOutSample(sid, pos);
    } else {
        qWarning() << QString("can NOT find slot group from group_id:%1 pos:%2").arg(group_id).arg(pos);
    }
}

void SlotsManager::slideStainStart(const QString &group_id, const QString &sid, int pos)
{
    if (mSlotGroupMap.contains(group_id)) {
        auto group = mSlotGroupMap.value(group_id);
        group->stainStart(sid, pos);
    } else {
        qWarning() << "can NOT find slot group from pos:" << pos << "pid:" << group_id;
    }

    updateRemainingTime(sid);
}

void SlotsManager::updateRemainingTime(const QString &sid)
{
    auto slide = RtSampleManager::GetInstance()->getSlide(sid);
    if (slide && slide->isStainEnable()) {
        int remain = getStainTimeRemaining(slide);
        RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Staining, remain);
    }
}

int SlotsManager::getStainTimeRemaining(QSharedPointer<RtSlide> slide)
{
    Q_ASSERT(slide);

    qDebug() << slide->slide_id() << "calculate remain time..";

    QString text;

    QStringList list = slide->processList();
    int sum = 0;

    for (int i = 0; i < list.count(); ++i) {
        QString procedure = list.at(i);
        if (procedure == "wash") {
            int washTime = mGroupWash->washCount() * 5;
            sum += washTime;
            text.append(QString("wash(%1s)+").arg(washTime));
        } else if (mSlotGroupMap.contains(procedure)) {
            auto group = mSlotGroupMap.value(procedure);
            int stainTime = group->duration();
            sum += stainTime;
            text.append(QString("%1(%2s)+").arg(group->groupid()).arg(stainTime));
        }

        /* per grip time */
        sum += 8;
        text.append("(8s)+");
    }
    text.chop(1);
    qDebug() << text;
    qDebug().noquote() << QString("%1 stain duration SUM remain:%2s").arg(slide->slide_id()).arg(sum);
    return sum;
}

int SlotsManager::applySlotPos(const QString &group_id)
{
    if (mSlotGroupMap.contains(group_id)) {
        auto pool = mSlotGroupMap.value(group_id);
        return pool->getVacantSlot();
    }
    return -1;
}

void SlotsManager::prepareSlotPos(int pos, const QString &sid)
{
    auto slide = RtSampleManager::GetInstance()->getSlide(sid);
    if (slide) {
        if (slide->isCancelled() == false) {
            foreach (SlotGroupBase *group, mSlotGroupMap) {
                if (group->contains(pos)) {
                    group->prepareSlotPos(pos, sid);
                    break;
                }
            }
        } else {
            if (mGroupWash->contains(pos)) {
                mGroupWash->prepareSlotPos(pos, sid);
            }
        }
    }
}

void SlotsManager::setSolutionExpiration(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString group = obj.value("group").toString();
        int expireTime = obj.value("expireTime").toInt();
        int expireCount = obj.value("expireCount").toInt();
        if (mSlotGroupMap.contains(group)) {
            auto slotGroup = mSlotGroupMap.value(group);
            slotGroup->setMaxStainCount(expireCount);
            slotGroup->setSolutionExpiryTime(expireTime * 60);
        }
    }
}

SlotBase *SlotsManager::slotAt(int pos)
{
    foreach (SlotGroupBase *group, mSlotGroupMap) {
        if (group->contains(pos)) {
            return group->slotAt(pos);
        }
    }
    return nullptr;
}

void SlotsManager::drainAllSlots_slot()
{
    switch (s_drainSlots) {
    case DrainSlotsState::Idle:
        if (m_isDrainSlotsFinished == false) {
            s_drainSlots = DrainSlotsState::Drain_Slot_C1;
        }
        break;

    case DrainSlotsState::Drain_Slot_C1:
        if (m_drain_slot_arr.isEmpty()) {
            mGroupC1->drainAllSlots();
        } else {
            mGroupC1->drainSlots(m_drain_slot_arr);
        }
        s_drainSlots = DrainSlotsState::WaitF_Drain_Slot_C1_Done;
        break;
    case DrainSlotsState::WaitF_Drain_Slot_C1_Done:
        if (mGroupC1->isDrainFinished()) {
            s_drainSlots = DrainSlotsState::Drain_Slot_C2;
        }
        break;

    case DrainSlotsState::Drain_Slot_C2:
        if (m_drain_slot_arr.isEmpty()) {
            mGroupC2->drainAllSlots();
        } else {
            mGroupC2->drainSlots(m_drain_slot_arr);
        }
        s_drainSlots = DrainSlotsState::WaitF_Drain_Slot_C2_Done;
        break;
    case DrainSlotsState::WaitF_Drain_Slot_C2_Done:
        if (mGroupC2->isDrainFinished()) {
            s_drainSlots = DrainSlotsState::Finish;
        }
        break;

    case DrainSlotsState::Finish:
        m_isDrainSlotsFinished = true;
        m_drainSlotsTimer->stop();
        if (m_drainSlots_id > 0) {
            FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, m_drainSlots_id);
            m_drainSlots_id = 0;
        }
        s_drainSlots = DrainSlotsState::Idle;
        break;
    }
}

void SlotsManager::cleanAllSlots_slot()
{
    switch (s_cleanSlots) {
    case CleanSlotsState::Idle:
        if (m_isCleanSlotsFinished == false) {
            s_cleanSlots = CleanSlotsState::Drain_Fix;
        }
        break;

    case CleanSlotsState::Drain_Fix:
        if (m_cleanFixArr.isEmpty() == false) {
            mGroupFix->drainSlots(m_cleanFixArr);
            s_cleanSlots = CleanSlotsState::WaitF_Drain_Fix_Done;
        } else {
            s_cleanSlots = CleanSlotsState::Clean_Slot_C1;
        }
        break;
    case CleanSlotsState::WaitF_Drain_Fix_Done:
        if (mGroupFix->isDrainFinished()) {
            m_cleanFixArr.clear();
            s_cleanSlots = CleanSlotsState::Clean_Slot_C1;
        }
        break;

    case CleanSlotsState::Clean_Slot_C1:
        if (m_cleanAllSlots == true) {
            mGroupC1->cleanAllSlots(m_detergent);
            s_cleanSlots = CleanSlotsState::WaitF_Clean_Slot_C1_Done;
        } else if (m_cleanC1Arr.isEmpty() == false) {
            mGroupC1->cleanSlots(m_cleanC1Arr, m_detergent);
            s_cleanSlots = CleanSlotsState::WaitF_Clean_Slot_C1_Done;
        } else {
            s_cleanSlots = CleanSlotsState::Clean_Slot_C2;
        }
        break;
    case CleanSlotsState::WaitF_Clean_Slot_C1_Done:
        if (mGroupC1->isCleanFinished()) {
            m_cleanC1Arr.clear();
            s_cleanSlots = CleanSlotsState::Clean_Slot_C2;
        }
        break;

    case CleanSlotsState::Clean_Slot_C2:
        if (m_cleanAllSlots == true) {
            mGroupC2->cleanAllSlots(m_detergent);
            s_cleanSlots = CleanSlotsState::WaitF_Clean_Slot_C2_Done;
        } else if (m_cleanC2Arr.isEmpty() == false) {
            mGroupC2->cleanSlots(m_cleanC2Arr, m_detergent);
            s_cleanSlots = CleanSlotsState::WaitF_Clean_Slot_C2_Done;
        } else {
            s_cleanSlots = CleanSlotsState::Drain_Wash;
        }
        break;
    case CleanSlotsState::WaitF_Clean_Slot_C2_Done:
        if (mGroupC2->isCleanFinished()) {
            m_cleanC2Arr.clear();
            s_cleanSlots = CleanSlotsState::Drain_Wash;
        }
        break;

    case CleanSlotsState::Drain_Wash:
        if (m_drainWash == true) {
            mGroupWash->drainAllSlots();
            s_cleanSlots = CleanSlotsState::WaitF_Drain_Wash_Done;
        } else {
            s_cleanSlots = CleanSlotsState::Finish;
        }
        break;
    case CleanSlotsState::WaitF_Drain_Wash_Done:
        if (mGroupWash->isDrainFinished()) {
            m_drainWash = false;
            s_cleanSlots = CleanSlotsState::Finish;
        }
        break;

    case CleanSlotsState::Finish:
        m_isCleanSlotsFinished = true;
        m_cleanSlotsTimer->stop();
        qDebug() << "Clean Slots Finished. id:" << m_cleanSlots_id;
        if (m_cleanSlots_id > 0) {
            FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, m_cleanSlots_id);
            m_cleanSlots_id = 0;
        }
        s_cleanSlots = CleanSlotsState::Idle;
        break;
    }
}


