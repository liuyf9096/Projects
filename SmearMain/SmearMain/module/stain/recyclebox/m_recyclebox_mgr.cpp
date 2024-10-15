#include "m_recyclebox_mgr.h"
#include "recycle_box.h"
#include "messagecenter/f_message_center.h"
#include "f_common.h"

MRecycleBoxMgr *MRecycleBoxMgr::GetInstance()
{
    static MRecycleBoxMgr *instance = nullptr;
    if (instance == nullptr) {
        instance = new MRecycleBoxMgr();
    }
    return instance;
}

MRecycleBoxMgr::MRecycleBoxMgr(QObject *parent)
    : DModuleBase("recycleBoxs", "recycleBoxs", parent)
    , m_boxid(0)
{
    IsReaderEnable = FCommon::GetInstance()->isReaderEnable();
    IsHeaterEnable = FCommon::GetInstance()->getConfigValue("stain", "dryfan", "enable").toBool();
    IsCheckSensorValue = FCommon::GetInstance()->getConfigValue("stain", "recycle_box", "check_manu_sensor").toBool();

    mMaxCount = 10;
    m_isclearMode = false;

    _setBoxRange();
    state_init();

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout, this, &MRecycleBoxMgr::onTimer_slot);

    m_heatTime_sec = 10;
    m_heatTimer = new QTimer(this);
    m_heatTimer->setSingleShot(true);
    m_heatTimer->setInterval(m_heatTime_sec * 1000);
    connect(m_heatTimer, &QTimer::timeout, this, [=](){
        qDebug() << "Heating Finish.";
        m_heating = false;
    });

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onReaderMessageResult_signal,
            this, &MRecycleBoxMgr::onReaderMessageResult_slot);

    qDebug() << "Stain:ReaderEnable:" << IsReaderEnable;
    qDebug() << "Stain:HeaterEnable:" << IsHeaterEnable;
}

void MRecycleBoxMgr::_setBoxRange()
{
    QJsonObject obj = FCommon::GetInstance()->getConfigValue("slot_setup", "standard", "stain_slot").toObject();
    if (obj.isEmpty()) {
        qFatal("Missing slot config file");
    }
    if (obj.contains("recycle")) {
        QJsonObject recycleObj = obj.value("recycle").toObject();
        mFrom = recycleObj.value("from").toInt();
        mTo = recycleObj.value("to").toInt();
        qDebug() << "set Recycle Box pos, from:" << mFrom << "to:" << mTo;
    }
}

void MRecycleBoxMgr::state_init()
{
    m_startRun = false;
    m_isNoBox = false;
    m_isBoxAvailable = false;
    m_boxState = Unknown;
    m_heating = false;
    m_waitforReader = 0;

    m_checkReaderFinished = true;
    m_reader_ready = true;

    mRecycleBox = nullptr;
    mDryBox = nullptr;

    s_fillBox = FillBoxState::Idle;
    s_dryBox = DryBoxState::Idle;
    s_pTManu = PushToManuState::Idle;
}

void MRecycleBoxMgr::renewBox()
{
    m_isNoBox = false;
}

void MRecycleBoxMgr::setBoxMaxCapacity(int count)
{
    if (count > 0 && count <= 10) {
        mMaxCount = count;
        if (mRecycleBox != nullptr) {
            mRecycleBox->setMaxCount(count);
        }
    }
}

void MRecycleBoxMgr::startRun(bool on)
{
    m_startRun = on;
    if (on) {
        renewBox();
    }
}

void MRecycleBoxMgr::setClearMode(bool on)
{
    m_isclearMode = on;
    qDebug() << "MRecycleBoxMgr:setClearMode:" << on;
}

bool MRecycleBoxMgr::ejectCurrentBox()
{
    auto box = getCurrentBox();
    if (box) {
        box->eject();
        return true;
    }
    return false;
}

bool MRecycleBoxMgr::isIdle()
{
    if (s_fillBox <= FillBoxState::Push_New_Box
            && s_dryBox == DryBoxState::Idle) {
        return true;
    }
    return false;
}

void MRecycleBoxMgr::setHeaterParams(const QJsonObject &obj)
{
    if (obj.contains("time")) {
        int time = obj.value("time").toInt();
        setHeatTime(time);
    }
    if (obj.contains("temp")) {
        int temp = obj.value("temp").toInt();
        dev->stain()->cmd_SetHeaterTempValue(0, temp);
        qDebug() << "Set Recycle Dry temp:" << temp << "`C";
    }
}

void MRecycleBoxMgr::setHeatTime(int sec)
{
    m_heatTime_sec = sec;
    m_heatTimer->setInterval(m_heatTime_sec * 1000);
    qDebug() << "Set Recycle Dry time:" << sec << "s";
}

QSharedPointer<RecycleBox> MRecycleBoxMgr::newRecycleBox()
{
    QString bid = QString("Box_%1").arg(m_boxid++);

    auto box = QSharedPointer<RecycleBox>(new RecycleBox(bid, mFrom, mTo, this));
    box->setMaxCount(mMaxCount);
    if (m_isclearMode == true) {
        box->setClearMode();
    }
    return box;
}

void MRecycleBoxMgr::start()
{
    m_timer->start();
}

void MRecycleBoxMgr::reset()
{
    state_init();
}

void MRecycleBoxMgr::stop()
{
    m_timer->stop();
}

QSharedPointer<RecycleBox> MRecycleBoxMgr::getCurrentBox()
{
    if (m_isBoxAvailable == true) {
        if (mRecycleBox && mRecycleBox->isFull() == false) {
            return mRecycleBox;
        }
    }
    return nullptr;
}

void MRecycleBoxMgr::onTimer_slot()
{
    if (dev->stain()->isResetOk() == false) {
        return;
    }

    receiveSlidesProcess();
    dryProcess();
    pushToManuProcess();
}

void MRecycleBoxMgr::receiveSlidesProcess()
{
    switch (s_fillBox) {
    case FillBoxState::Idle:
        if (m_startRun == true) {
            logProcess("FillBox", 1, "Get New RecycleBox");
            s_fillBox = FillBoxState::Push_New_Box;
        }
        break;

    case FillBoxState::Push_New_Box:
    {
        bool ok = dev->stain()->cmd_NewRecycleBox();
        if (ok) {
            logProcess("FillBox", 2, "NewRecycleBox", "Try to Push New Box.");
            s_fillBox = FillBoxState::WaitF_Push_New_Box_Done;
        }
    }
        break;
    case FillBoxState::WaitF_Push_New_Box_Done:
        if (dev->stain()->getFuncResult("NewRecycleBox") == Func_Done) {
            m_boxState = Available;
            m_isBoxAvailable = true;

            /* Generate New Box */
            mRecycleBox = newRecycleBox();
            logProcess("FillBox", 3, "NewRecycleBox", mRecycleBox->bid());

            ExceptionCenter::GetInstance()->sendClearExceptionMessage("stain_NewRecycleBox_80");

            s_fillBox = FillBoxState::Check_Box_Count;
        } else if (dev->stain()->getFuncResult("NewRecycleBox") == Func_Fail) {
            m_boxState = None;
            logProcess("FillBox", 3, "NewRecycleBox", "Check For New Box.");
            s_fillBox = FillBoxState::Check_For_Vacant_Box;
        }
        break;

    case FillBoxState::Check_For_Vacant_Box:
        dev->stain()->setCheckSensorEnable(mModuleId, true);
        m_boxState = Unknown;
        s_fillBox = FillBoxState::WaitF_Sensor_Res1;
        break;
    case FillBoxState::WaitF_Sensor_Res1:
        if (dev->stain()->checkSensorValueOr("import1", "import2") == true) {
            dev->stain()->setCheckSensorEnable(mModuleId, false);
            s_fillBox = FillBoxState::Push_New_Box;
        } else {
            m_boxState = None;
        }
        break;

    case FillBoxState::Check_Box_Count:
        dev->stain()->cmd_CheckSensorValue();
        logProcess("FillBox", 4, "NewRecycleBox", "Get Available new box count.");
        s_fillBox = FillBoxState::WaitF_Sensor_Res2;
        break;
    case FillBoxState::WaitF_Sensor_Res2:
        if (dev->stain()->isFuncDone("CheckSensorValue")) {
            int count = currentSpareBoxCount() + 1;
            qDebug() << "Current Spare Box Count:" << count;
            currentRecycleBoxCount();

            JPacket p(PacketType::Notification);
            p.module = "RecycleBox";
            p.api = "CurrentSpareBoxCount";
            p.paramsValue = QJsonObject({{"count", count}});
            FMessageCenter::GetInstance()->sendUIMessage(p);

            s_fillBox = FillBoxState::WaitF_Box_Full;
        }
        break;

    case FillBoxState::WaitF_Box_Full:
        if (mRecycleBox->isFull() || mRecycleBox->isEjected()) {
            m_boxState = Unknown;
            m_isBoxAvailable = false;
            logProcess("FillBox", 5, QString("%1 is full.").arg(mRecycleBox->bid()));
            s_fillBox = FillBoxState::Dry_Check_Sensor_Value;
        }
        break;

    case FillBoxState::Dry_Check_Sensor_Value:
        dev->stain()->cmd_CheckSensorValue();
        s_fillBox = FillBoxState::WaitF_Sensor_Res3;
        break;
    case FillBoxState::WaitF_Sensor_Res3:
        if (dev->stain()->isFuncDone("CheckSensorValue")) {
            if (currentRecycleBoxCount() < 6) {
                logProcess("FillBox", 6, "Move box to Dry place.");
                s_fillBox = FillBoxState::Move_To_Dry_Pos;
            } else {
                s_fillBox = FillBoxState::Finish;
            }
        }
        break;

    case FillBoxState::Move_To_Dry_Pos:
    {
        bool ok = dev->stain()->cmd_RecycleBox_ToDryPos();
        if (ok) {
            s_fillBox = FillBoxState::WaitF_Move_To_Dry_Pos_Done;
        }
    }
        break;
    case FillBoxState::WaitF_Move_To_Dry_Pos_Done:
        if (dev->stain()->isFuncDone("RecycleBox_ToDryPos")) {
            mRecycleBox->unloadBox();
            m_boxState = None;
            mDryBox = mRecycleBox;
            s_fillBox = FillBoxState::Finish;
        }
        break;

    case FillBoxState::Finish:
        logProcess("FillBox", 99, "Finished.");
        mRecycleBox = nullptr;
        s_fillBox = FillBoxState::Idle;
        break;
    }
}

void MRecycleBoxMgr::dryProcess()
{
    switch (s_dryBox) {
    case DryBoxState::Idle:
        if (mDryBox != nullptr) {
            if (mDryBox->isClearMode()) {
                logProcess("DryBox", 1, "Start Clear Box", mDryBox->bid());
                s_dryBox = DryBoxState::PushOut_Box;
            } else if (mDryBox->containsStainSlide()) {
                logProcess("DryBox", 1, "Start Dry Box", mDryBox->bid());
                if (IsHeaterEnable) {
                    s_dryBox = DryBoxState::Start_Heater;
                } else {
                    s_dryBox = DryBoxState::PushOut_Box;
                }
            } else {
                logProcess("DryBox", 1, "Start", mDryBox->bid());
                s_dryBox = DryBoxState::PushOut_Box;
            }
        }
        break;

    case DryBoxState::Start_Heater:
        m_heating = true;
        m_heatTimer->start();
        dev->stain()->cmd_Heater_Open();
        s_dryBox = DryBoxState::WaitF_Dry_Done;
        break;
    case DryBoxState::WaitF_Start_Heater_On_Done:
        if (dev->stain()->isFuncDone("Heater_Open")) {
            s_dryBox = DryBoxState::WaitF_Dry_Done;
        }
        break;

    case DryBoxState::WaitF_Dry_Done:
        if (m_heating == false) {
            dev->stain()->cmd_Heater_Close();
            s_dryBox = DryBoxState::PushOut_Box;
        }
        break;

    case DryBoxState::PushOut_Box:
        dev->stain()->cmd_CheckSensorValue();
        logProcess("DryBox", 2, "PushOut_Box", mDryBox->bid());
        s_dryBox = DryBoxState::WaitF_Sensor_Res;
        break;

    case DryBoxState::WaitF_Sensor_Res:
        if (dev->stain()->isFuncDone("CheckSensorValue")) {
            if (mDryBox->isClearMode() || mDryBox->isEmpty())
            {
                s_dryBox = DryBoxState::Push_To_Manu_Pos;
                break;
            }
            else if (mDryBox->allStainSlide())
            {
                qDebug() << "Dry box allStainSlide.";
                if (IsReaderEnable == true) {
                    if (IsCheckSensorValue == true) {
                        if (dev->stain()->checkSensorValue("export1") == false
                                && dev->stain()->checkSensorValue("export2") == false
                                && dev->stain()->checkSensorValue("export3") == false
                                && dev->stain()->checkSensorValue("export4") == false) {
                            m_waitforReader = 0;
                            s_dryBox = DryBoxState::Check_Reader;
                        }
                    } else {
                        m_waitforReader = 0;
                        s_dryBox = DryBoxState::Check_Reader;
                    }
                    break;
                }
            }

            if (dev->stain()->checkSensorValue("export6") == false) {
                s_dryBox = DryBoxState::Push_To_Manu_Pos;
            } else {
                s_dryBox = DryBoxState::Finish;
            }
        }
        break;

    case DryBoxState::Push_To_Manu_Pos:
#if 1
        mPushTManuList.append(mDryBox);
        s_dryBox = DryBoxState::Finish;
#else
    {
        bool ok = dev->stain()->cmd_RecycleBox_ToManualPos();
        if (ok) {
            s_dryBox = DryBoxState::WaitF_PushTo_ManuPos_Done;
        }
    }
#endif
        break;
    case DryBoxState::WaitF_PushTo_ManuPos_Done:
        if (dev->stain()->getFuncResult("RecycleBox_ToManualPos") == Func_Done) {
            logProcess("DryBox", 3, QString("%1 is delivered.").arg(mDryBox->bid()));
            mDryBox->sendBoxToManu();
            s_dryBox = DryBoxState::Finish;
        }
        break;

    case DryBoxState::Check_Reader:
        if (FMessageCenter::GetInstance()->isReaderConnected()) {
            JPacket p("Reader", "Import", "isReceiveSlideBoxReady");
            FMessageCenter::GetInstance()->sendReaderMessage(p);
            m_checkReaderFinished = false;
            s_dryBox = DryBoxState::PushToReader;
        } else if (m_waitforReader < 10 * 50) {
            m_waitforReader++;
        } else {
            s_dryBox = DryBoxState::PushToReader;
        }
        break;
    case DryBoxState::PushToReader:
        if (m_checkReaderFinished == true) {
            if (m_reader_ready == true) {
                bool ok = dev->stain()->cmd_RecycleBox_ToReader();
                if (ok) {
                    s_dryBox = DryBoxState::WaitF_PushToReader_Done;
                }
            } else {
                s_dryBox = DryBoxState::Push_To_Manu_Pos;
            }
        } else if (FMessageCenter::GetInstance()->isReaderConnected() == false) {
            s_dryBox = DryBoxState::Push_To_Manu_Pos;
        }
        break;
    case DryBoxState::WaitF_PushToReader_Done:
        if (dev->stain()->getFuncResult("RecycleBox_ToReader") == Func_Done) {
            logProcess("DryBox", 3, QString("%1 is delivered.").arg(mDryBox->bid()));
            mDryBox->sendBoxToReader();
            mDryBox->sendReaderSlideInfo();

            s_dryBox = DryBoxState::Finish;
        }
        break;

    case DryBoxState::Finish:
        logProcess("DryBox", 99, "Finished");
        mDryBox = nullptr;
        s_dryBox = DryBoxState::Idle;
        break;
    }
}

void MRecycleBoxMgr::pushToManuProcess()
{
    switch (s_pTManu) {
    case PushToManuState::Idle:
        if (mPushTManuList.isEmpty() == false) {
            if (IsCheckSensorValue == true) {
                logProcess("PushOut", 1, "Check Manu Sensor.");
                s_pTManu = PushToManuState::Check_Manu_Sensor;
            } else {
                logProcess("PushOut", 1, "Push to Manu Pos.");
                s_pTManu = PushToManuState::Push_To_Manu_Pos;
            }
        }
        break;

    case PushToManuState::Check_Manu_Sensor:
        dev->stain()->cmd_CheckSensorValue();
        s_pTManu = PushToManuState::WaitF_Sensor_Res;
        break;
    case PushToManuState::WaitF_Sensor_Res:
        if (dev->stain()->isFuncDone("CheckSensorValue")) {
            if (dev->stain()->checkSensorValue("export6") == false) {
                logProcess("PushOut", 2, "Push to Manu Pos.");
                s_pTManu = PushToManuState::Push_To_Manu_Pos;
            } else {
                s_pTManu = PushToManuState::Check_Manu_Sensor;
            }
        }
        break;

    case PushToManuState::Push_To_Manu_Pos:
    {
        bool ok = dev->stain()->cmd_RecycleBox_ToManualPos();
        if (ok) {
            s_pTManu = PushToManuState::WaitF_PushTo_ManuPos_Done;
        }
    }
        break;
    case PushToManuState::WaitF_PushTo_ManuPos_Done:
        if (dev->stain()->isFuncDone("RecycleBox_ToManualPos")) {
            auto box = mPushTManuList.takeFirst();
            box->sendBoxToManu();
            s_pTManu = PushToManuState::Finish;
        }
        break;

    case PushToManuState::Finish:
        logProcess("PushOut", 99, "Finished.");
        s_pTManu = PushToManuState::Idle;
        break;
    }
}

int MRecycleBoxMgr::currentSpareBoxCount()
{
    int count = 0;
    for (int i = 6; i > 0; --i) {
        QString sensor = QString("import%1").arg(i);
        if (dev->stain()->checkSensorValue(sensor) == true) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int MRecycleBoxMgr::currentRecycleBoxCount()
{
    int count = 0;
    for (int i = 1; i < 6; ++i) {
        QString sensor = QString("export%1").arg(i + 1);
        if (dev->stain()->checkSensorValue(sensor) == true) {
            count++;
        }
    }
    qDebug() << "Current Recycle Box Count:" << count;
    return count;
}

void MRecycleBoxMgr::onReaderMessageResult_slot(const JPacket &result, const JPacket &request)
{
    if (request.api == "isReceiveSlideBoxReady") {
        m_checkReaderFinished = true;
        m_reader_ready = result.resValue.toBool();
    } else if (request.api == "AddNewSlideBox") {
        //..
    }
}
