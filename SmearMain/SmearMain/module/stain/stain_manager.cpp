#include "stain_manager.h"
#include "gripper/m_gripper1.h"
#include "gripper/m_gripper2.h"
#include "f_common.h"

#include <QTimer>
#include <QDebug>

StainManager *StainManager::GetInstance()
{
    static StainManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new StainManager();
    }
    return instance;
}

StainManager::StainManager(QObject *parent)
    : MManagerBase{"stain", parent}
{
    /* create */
    mRecycleBoxes = MRecycleBoxMgr::GetInstance();
    mStainCart = new MStainCart("staincart", this);
    mGrippers = GripperManager::GetInstance();
    mSlotManager = SlotsManager::GetInstance();
    mInfuser = new SolutionInfuser("infuser", this);
    mDrainer = new SolutionDrainer("drainer", this);

    /* start point */
    pImport = new StainImport("import", 26, this);
    pStainOnly1 = new StainOnly1("stainonly1", 11, 20);
    pStainOnly2 = new StainOnly2("stainonly2", 1, 10);

    /* settting */
    mGrippers->mG2->setRecycleBoxMgr(mRecycleBoxes);
    mStainCart->setStainImport(pImport);
    mSlotManager->mGroupFix->setDrainer(mDrainer);
    mSlotManager->mGroupC1->setInfuser(mInfuser);
    mSlotManager->mGroupC1->setDrainer(mDrainer);
    mSlotManager->mGroupC2->setInfuser(mInfuser);
    mSlotManager->mGroupC2->setDrainer(mDrainer);

    addModule(mStainCart, mRecycleBoxes);
    addModule(mGrippers->mG1, mGrippers->mG2);
    addModule(pImport);
    addModule(pStainOnly1, pStainOnly2);
    addModule(mInfuser, mDrainer);

    /* check new timer */
    mCheckNewTimer = new QTimer(this);
    int check_new_sec = FCommon::GetInstance()->getConfigValue("stain", "import_check_new").toInt();
    mCheckNewTimer->setInterval(check_new_sec * 1000);
    qDebug() << "Stain Import Default Check Interval:" << check_new_sec << "s";
    connect(mCheckNewTimer, &QTimer::timeout, this, &StainManager::onCheckNewTimer_slot);

    connect(pImport, &StainImport::onSendRequest_singal, this, &StainManager::handleSlotTimeout_slot);
    connect(pStainOnly1, &StainOnlyBase::onSendRequest_singal, this, &StainManager::handleSlotTimeout_slot);
    connect(pStainOnly2, &StainOnlyBase::onSendRequest_singal, this, &StainManager::handleSlotTimeout_slot);

    connect(this, &StainManager::onGripperRequest_signal,
            mGrippers, &GripperManager::onGripperRequest_slot);
}

void StainManager::takingOutSample(const QString &group_id, const QString &sid, int pos)
{
    qDebug() << __FUNCTION__ << group_id << sid << pos;
}

void StainManager::takeOutSample(const QString &group_id, const QString &sid, int pos)
{
    if (group_id == "import") {
        pImport->takeOutSample(sid, pos);
    } else if (group_id == "stainonly1") {
        pStainOnly1->takeOutSample(sid, pos);
    } else if (group_id == "stainonly2") {
        pStainOnly2->takeOutSample(sid, pos);
    } else {
        mSlotManager->takeOutSample(group_id, sid, pos);
    }
}

void StainManager::setStainParams(const QJsonObject &obj)
{
    if (obj.contains("method")) {
        QString method = obj.value("method").toString();
        mSlotManager->setStainMethod(method);
    }
    if (obj.contains("process")) {
        QJsonArray processArr = obj.value("process").toArray();
        mSlotManager->setSlotParams(processArr);
    }
    if (obj.contains("period")) {
        int period = obj.value("period").toInt();
        setCheckTimerInterval(period);
    }
}

void StainManager::setCheckTimerInterval(int sec)
{
    if (sec > 0 && sec < 180) {
        mCheckNewTimer->setInterval(sec * 1000);
        qInfo().noquote() << QString("stain Check interval:%1s").arg(sec);
    }
}

void StainManager::startCheckTimer(bool firstRun)
{
    if (mCheckNewTimer->isActive() == false) {
        mCheckNewTimer->start();

        if (firstRun) {
            onCheckNewTimer_slot();
        }
        MRecycleBoxMgr::GetInstance()->startRun(true);
    }
}

void StainManager::needCheckImmediately()
{
    QTimer::singleShot(200, this, &StainManager::onCheckNewTimer_slot);
}

void StainManager::startStainOnlyProcess(quint64 id, const QJsonObject &obj)
{
    QString stainBox = obj.value("stainBox").toString();
    QJsonArray arr = obj.value("slides").toArray();
    if (stainBox == "left") {
        addStainOnlyRequest(id, 2, arr);
    } else if (stainBox == "right") {
        addStainOnlyRequest(id, 1, arr);
    }
}

bool StainManager::addStainOnlyRequest(quint64 id, int index, const QJsonArray &arr)
{
    if (arr.isEmpty()) {
        return false;
    }

    StainOnlyBase *stainOnly = nullptr;
    if (index == 1) {
        stainOnly = pStainOnly1;
    } else if (index == 2) {
        stainOnly = pStainOnly2;
    }
    if (stainOnly) {
        if (stainOnly->isEmpty()) {
            stainOnly->loadSlides(id, arr);

            startCheckTimer(true);
            return true;
        }
    }
    return false;
}

void StainManager::cancelStainOnlyRequest()
{
    pStainOnly1->unloadAllSlides();
    pStainOnly2->unloadAllSlides();
}

void StainManager::stop()
{
    MManagerBase::stop();
}

void StainManager::onCheckNewTimer_slot()
{
    qInfo() << "[!] Check Unstain Slide...";

    if (pImport->hasRequest()) {
        pImport->sendRequest();
        qDebug() << "send import request.";
    } else if (pStainOnly1->hasRequest()) {
        pStainOnly1->sendRequest();
        qDebug() << "send stain-only request." << pStainOnly1->userid();
    } else if (pStainOnly2->hasRequest()) {
        pStainOnly2->sendRequest();
        qDebug() << "send stain-only request." << pStainOnly2->userid();
    }
}

void StainManager::handleSlotTimeout_slot(const QString &from_groupid, int from_pos, const QString &sid)
{
    QJsonObject obj;
    obj.insert("from_groupid", from_groupid);
    obj.insert("from_pos", from_pos);
    obj.insert("sid", sid);

    auto slide = RtSampleManager::GetInstance()->getSlide(sid);
    if (slide) {
        QString to_groupid = slide->getNextStainProcess();
        if (!to_groupid.isEmpty()) {
            obj.insert("to_groupid", to_groupid);
        } else {
            obj.insert("to_groupid", "recycleBox");
        }
        emit onGripperRequest_signal(obj);
    }
    qDebug() << "Gripper Request:" << obj;
}
