#include "m_station.h"
#include "sample/rt_sample_manager.h"
#include "messagecenter/f_message_center.h"
#include "rack/rt_rack.h"
#include "cart/m_cart.h"
#include "station_manager.h"
#include "f_common.h"

MStation::MStation(const QString &mid, const QString &userid, int posscan, int postest, QObject *parent)
    : DModuleBase(mid, userid, parent)
    , Pos_Scan(posscan)
    , Pos_Test(postest)
{
    m_isConnected = false;
    m_isUnited = false;

    closetOpen_id = 0;
    closetClose_id = 0;
    m_scanBarDuration = FCommon::GetInstance()->getConfigValue("general", "scan_barcode_ms").toInt(5);

    _timer_init();
    state_init();

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onClientDeviceConnected_signal,
            this, &MStation::handleConnect_slot);
    connect(center, &FMessageCenter::onClientsMessageRequestPacket_signal,
            this, &MStation::handleRequestMessage_slot);
    connect(center, &FMessageCenter::onClientsMessageResultPacket_signal,
            this, &MStation::handleResultMessage_slot);
    connect(center, &FMessageCenter::onClientsMessageErrorPacket_signal,
            this, &MStation::handleErrorMessage_slot);
}

void MStation::_timer_init()
{
    m_ScanTimer = new QTimer(this);
    m_ScanTimer->setInterval(50);
    connect(m_ScanTimer, &QTimer::timeout, this, &MStation::onScanTimer_slot);

    m_ReceiveTimer = new QTimer(this);
    m_ReceiveTimer->setInterval(50);
    connect(m_ReceiveTimer, &QTimer::timeout, this, &MStation::onReceiveTimer_slot);

    m_RecycleTimer = new QTimer(this);
    m_RecycleTimer->setInterval(50);
    connect(m_RecycleTimer, &QTimer::timeout, this, &MStation::onRecycleTimer_slot);

    m_EmergencyTimer = new QTimer(this);
    m_EmergencyTimer->setInterval(50);
    connect(m_EmergencyTimer, &QTimer::timeout, this, &MStation::onEmergencyTimer_slot);
}

void MStation::state_init()
{
    m_isScanFinished = true;
    mScanPos = -1;
    mScanRack = nullptr;
    mScanSample = nullptr;
    s_Scan = ScanState::Idle;

    m_isReceiveFinished = true;
    m_receiveSample = nullptr;
    s_Receive = ReceiveState::Idle;

    m_isRecycleFinished = true;
    m_recycleSample = nullptr;
    s_Recycle = RecycleState::Idle;

    m_isStaionCheckIdleFinished = true;
    m_isStationRotateTubeFinished = true;
    m_isStationRotateTubeError = false;
    m_isStationTakeUpNewSampleFinished = true;
    m_isStationRecycleSampleFinished = true;

    s_open = ClosetState::Idle;
    m_isClosetOpenFinish = true;
    s_close = ClosetState::Idle;
    m_isClosetCloseFinish = true;
}

void MStation::setClientId(const QString &clientid)
{
    mClientid = clientid;
    qInfo().noquote() << QString("Staion[%1] cliend_id: '%2'").arg(mUserId, clientid);

    /* record */
    mRecordDb->updateRecord("line_device", {{"address", mClientid}}, {{"device_id", mDevid}});

    StationManager::GetInstance()->updateStationStatus();
}

void MStation::setUnited(bool isUnited)
{
    m_isUnited = isUnited;
    qDebug() << QString("Station [%1] setUnited:").arg(mDevid) << isUnited;

    /* record */
    mRecordDb->updateRecord("line_device", {{"isUnited", m_isUnited}}, {{"device_id", mDevid}});

    StationManager::GetInstance()->updateStationStatus();
}

void MStation::start()
{
    m_ScanTimer->start();
    m_ReceiveTimer->start();
    m_RecycleTimer->start();
    m_EmergencyTimer->start();

    DModuleBase::start();
}

void MStation::reset()
{
    state_init();
    DModuleBase::reset();
}

void MStation::stop()
{
    m_ScanTimer->stop();
    m_ReceiveTimer->stop();
    m_RecycleTimer->stop();
    m_EmergencyTimer->stop();
    DModuleBase::stop();
}

bool MStation::scanSampleBarcode(QSharedPointer<RtRack> rack, int pos)
{
    if (s_Scan == ScanState::Idle) {
        if (pos >= 0 && pos < 10) {
            mScanRack = rack;
            mScanPos = pos;
            mScanBarcode.clear();
            m_isScanFinished = false;
            return true;
        }
    } else {
        qWarning() << "scan Sample Barcode error";
    }
    return false;
}

bool MStation::checkTestAvailable(QSharedPointer<RtSample> sample)
{
    if (sample && m_isStaionCheckIdleFinished == true) {
        sendStationCheckIdle(sample);
        return true;
    }
    return false;
}

bool MStation::isTestAvailable(const QString &sid)
{
    if (m_checkAvailableMap.contains(sid)) {
        return m_checkAvailableMap.take(sid);
    }
    return false;
}

void MStation::onScanTimer_slot()
{
    if (m_isConnected == false) {
        return;
    }

    switch (s_Scan) {
    case ScanState::Idle:
        if (mScanRack != nullptr && mScanPos >= 0 && mScanPos < 10) {
            s_Scan = ScanState::Check_Sensor;
        }
        break;

    case ScanState::Check_Sensor:
        cmd_CheckSensorValue();
        s_Scan = ScanState::WaitF_Sensor_Value;
        break;

    case ScanState::WaitF_Sensor_Value:
        if (isCheckSensorValue_Done() == true) {
            s_Scan = ScanState::Check_Tube_Exist;
        }
        break;

    case ScanState::Check_Tube_Exist:
        if (isSampleExist() == true) {
            mScanSample = mScanRack->addNewSample(mScanPos);
            s_Scan = ScanState::Scan_Tube;
        } else {
            mScanRack->setSampleExist(mScanPos, false);
            s_Scan = ScanState::Finish;//todo
        }
        break;

    case ScanState::Scan_Tube:
        cmd_Scan_Open(m_scanBarDuration);
        sendStationRotateTube(m_scanBarDuration);
        s_Scan = ScanState::WaitF_Rotate_Tube_Finished;
        break;

    case ScanState::WaitF_Rotate_Tube_Finished:
        if (m_isStationRotateTubeFinished == true && isScanOpen_Done()) {
            if (m_isStationRotateTubeError == false) {
                qDebug() << "Rotate And Scan Finished.";

                mScanSample->setState(SampleStatus::Scaned, 1);
                mScanSample->setBarcode(mScanBarcode);
                mScanSample->setSampleID(mScanBarcode);

                /* Request Order-UID from DMU */
                RtSampleManager::GetInstance()->requestSampleOrderUid(mScanSample);

                s_Scan = ScanState::Finish;
            } else {
                cmd_Scan_Close();
                s_Scan = ScanState::Scan_Tube;
            }
        }
        break;

    case ScanState::Finish:
        mScanRack = nullptr;
        mScanSample = nullptr;
        mScanPos = -1;
        m_isScanFinished = true;
        s_Scan = ScanState::Idle;
        break;
    }
}

bool MStation::receiveNewSample(QSharedPointer<RtSample> sample)
{
    if (m_isReceiveFinished == true) {
        m_isReceiveFinished = false;
        m_receiveSample = sample;
        return true;
    } else {
        return false;
    }
}

bool MStation::recycleSample(QSharedPointer<RtSample> sample)
{
    if (m_isRecycleFinished == true) {
        m_isRecycleFinished = false;
        m_recycleSample = sample;
        return true;
    } else {
        return false;
    }
}

void MStation::onReceiveTimer_slot()
{
    if (m_isConnected == false) {
        return;
    }

    switch (s_Receive) {
    case ReceiveState::Idle:
        if (m_receiveSample && m_isReceiveFinished == false) {
            logProcess("Receive", 1, "Idle", QString("receive sample:%1").arg(m_receiveSample->sid()));
            s_Receive = ReceiveState::Send_Receive_Sample_Cmd;
        }
        break;

    case ReceiveState::Send_Receive_Sample_Cmd:
        sendStationTakeUpNewSample(m_receiveSample);
        s_Receive = ReceiveState::WaitF_Receive_Sample_Cmd_Done;
        break;
    case ReceiveState::WaitF_Receive_Sample_Cmd_Done:
        if (m_isStationTakeUpNewSampleFinished == true) {
            receiveFinished(m_receiveSample);
            s_Receive = ReceiveState::Finish;
        }
        break;

    case ReceiveState::Finish:
        logProcess("Receive", 99, "Finish", QString("sample:%1").arg(m_receiveSample->sid()));
        m_receiveSample = nullptr;
        m_isReceiveFinished = true;
        s_Receive = ReceiveState::Idle;
        break;
    }
}

void MStation::onRecycleTimer_slot()
{
    if (m_isConnected == false) {
        return;
    }

    switch (s_Recycle) {
    case RecycleState::Idle:
        if (m_recycleSample && m_isRecycleFinished == false) {
            logProcess("Recycle", 1, "Idle", QString("recycle sample:%1").arg(m_recycleSample->sid()));
            s_Recycle = RecycleState::Send_Recycle_Sample_Cmd;
        }
        break;

    case RecycleState::Send_Recycle_Sample_Cmd:
        sendStationRecycleSampleToRack(m_recycleSample);
        s_Recycle = RecycleState::WaitF_Recycle_Sample_Cmd_Done;
        break;
    case RecycleState::WaitF_Recycle_Sample_Cmd_Done:
        if (m_isStationRecycleSampleFinished == true) {
            recycleFinished(m_recycleSample);
            s_Recycle = RecycleState::Finish;
        }
        break;

    case RecycleState::Finish:
        logProcess("Recycle", 99, "Finish", QString("sample:%1").arg(m_recycleSample->sid()));
        m_recycleSample = nullptr;
        m_isRecycleFinished = true;
        s_Recycle = RecycleState::Idle;
        break;
    }
}

void MStation::onEmergencyTimer_slot()
{
    operateClosetOpen();
    operateClosetClose();
}

void MStation::operateClosetOpen()
{
    switch (s_open) {
    case ClosetState::Idle:
        if (m_isClosetOpenFinish == false) {
            s_open = ClosetState::Operate;
        }
        break;
    case ClosetState::Operate:
    {
        bool ok = cmd_Emergency_Open();
        if (ok) {
            s_open = ClosetState::WaitF_Operate_Done;
        }
    }
        break;
    case ClosetState::WaitF_Operate_Done:
        if (isEmergencyOpen_Done() == true) {
            s_open = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_isClosetOpenFinish = true;
    {
        JPacket p(PacketType::Result, closetOpen_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    }
        s_open = ClosetState::Idle;
        break;
    }
}

void MStation::operateClosetClose()
{
    switch (s_close) {
    case ClosetState::Idle:
        if (m_isClosetCloseFinish == false) {
            s_close = ClosetState::Operate;
        }
        break;
    case ClosetState::Operate:
    {
        bool ok = cmd_Emergency_Close();
        if (ok) {
            s_close = ClosetState::WaitF_Operate_Done;
        }
    }
        break;
    case ClosetState::WaitF_Operate_Done:
        if (isEmergencyClose_Done() == true) {
            s_close = ClosetState::Finish;
        }
        break;

    case ClosetState::Finish:
        m_isClosetCloseFinish = true;
    {
        JPacket p(PacketType::Result, closetClose_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    }
        s_close = ClosetState::Idle;
        break;
    }
}

void MStation::sendStationCheckIdle(QSharedPointer<RtSample> sample)
{
    m_isStaionCheckIdleFinished = false;

    m_checkAvailableMap.insert(sample->sid(), false);

    JPacket p(PacketType::Request);
    p.device = mDevid;
    p.module = "Sample";
    p.api = "IsReceiveNewSampleAble";

    QJsonObject obj;
    obj.insert("analysis_pattern", sample->testMode());
    obj.insert("blood_sample_pattern", sample->bloodtype());
    obj.insert("rack_id", sample->rack_id());
    obj.insert("rack_pos", sample->rack_pos());
    p.paramsValue = sample->getOrderInfo();

    quint64 id = FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);

    m_queryMap.insert(id, sample->sid());
}

void MStation::sendStationRotateTube(int duration)
{
    m_isStationRotateTubeFinished = false;
    m_isStationRotateTubeError = false;

    JPacket p(PacketType::Request);
    p.device = mDevid;
    p.module = "Sample";
    p.api = "RotateTube";

    QJsonObject obj;
    obj.insert("duration", duration);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
}

void MStation::sendStationTakeUpNewSample(QSharedPointer<RtSample> sample)
{
    m_isStationTakeUpNewSampleFinished = false;

    JPacket p(PacketType::Request);
    p.device = mDevid;
    p.module = "Sample";
    p.api = "TakeUpNewSample";

    QJsonObject obj;
    obj.insert("test_uid", sample->test_uid());
    obj.insert("order", sample->getOrderInfo());
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
}

void MStation::sendStationRecycleSampleToRack(QSharedPointer<RtSample> sample)
{
    if (sample == nullptr) {
        return;
    }

    m_isStationRecycleSampleFinished = false;

    JPacket p(PacketType::Request);
    p.device = mDevid;
    p.module = "Sample";
    p.api = "RecycleSampleToRack";

    QJsonObject obj;
    obj.insert("test_uid", sample->test_uid());
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
}

void MStation::handleConnect_slot(const QString &client_dev, const QString &client_id, bool connected)
{
    if (client_dev != mDevid) {
        return;
    }

    if (connected == true) {
        mClientid = client_id;
        m_isConnected = true;

        qInfo().noquote() << QString("Staion[%1] cliend_id: '%2' connected.").arg(mUserId, client_id);
    } else {
        m_isConnected = false;
        qInfo().noquote() << QString("Staion[%1] disconnected.").arg(mUserId);
        mClientid.clear();

        m_isStationRotateTubeFinished = true;
        m_isStationRotateTubeError = true;

        m_isStaionCheckIdleFinished = true;

        m_isStationTakeUpNewSampleFinished = true;
        m_isStationRecycleSampleFinished = true;
    }

    /* record */
    mRecordDb->updateRecord("line_device", {{"clientid", mClientid}}, {{"device_id", mDevid}});

    StationManager::GetInstance()->updateStationStatus();
}

void MStation::handleRequestMessage_slot(const QString &client_id, const JPacket &request)
{
    if (client_id != mClientid) {
        return;
    }

    if (request.module == "Emergency") {
        if (request.api == "ClosetOpen") {
            if (m_isClosetOpenFinish == true && s_open == ClosetState::Idle) {
                m_isClosetOpenFinish = false;
                closetOpen_id = request.id;
            }
        } else if (request.api == "ClosetClose") {
            if (m_isClosetCloseFinish == true && s_close == ClosetState::Idle) {
                m_isClosetCloseFinish = false;
                closetClose_id = request.id;
            }
        }
    }
}

void MStation::handleResultMessage_slot(const QString &client_id, const JPacket &result, const JPacket &request)
{
    if (client_id != mClientid) {
        return;
    }

    if (request.api == "IsReceiveNewSampleAble") {
        if (m_queryMap.contains(result.id)) {
            QString sid = m_queryMap.value(result.id);
            m_isStaionCheckIdleFinished = true;
            m_checkAvailableMap.insert(sid, result.resValue.toBool());
            qDebug() << "check: "<< m_checkAvailableMap;
        } else {
            qDebug() << "check fall." << m_checkAvailableMap;
        }
    } else if (request.api == "RotateTube") {
        m_isStationRotateTubeFinished = true;
        m_isStationRotateTubeError = false;
    } else if (request.api == "TakeUpNewSample") {
        m_isStationTakeUpNewSampleFinished = true;
    } else if (request.api == "RecycleSampleToRack") {
        m_isStationRecycleSampleFinished = true;
    }
}

void MStation::handleErrorMessage_slot(const QString &client_id, const JPacket &error, const JPacket &request)
{
    if (client_id != mClientid) {
        return;
    }

    if (request.api == "IsReceiveNewSampleAble") {
        if (m_queryMap.contains(error.id)) {
            QString sid = m_queryMap.value(error.id);
            m_isStaionCheckIdleFinished = true;
            m_checkAvailableMap.insert(sid, false);
            qDebug() << "check: "<< m_checkAvailableMap;
        } else {
            qDebug() << "check fall." << m_checkAvailableMap;
        }
    } else if (request.api == "RotateTube") {
        m_isStationRotateTubeFinished = true;
        m_isStationRotateTubeError = true;
    } else if (request.api == "TakeUpNewSample") {
//        m_isStationTakeUpNewSampleFinished = true;
    } else if (request.api == "RecycleSampleToRack") {
//        m_isStationRecycleSampleFinished = true;
    }
}

