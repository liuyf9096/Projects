#include "station_manager.h"
#include "m_station1.h"
#include "m_station_smear.h"
#include "messagecenter/f_message_center.h"

StationManager *StationManager::GetInstance()
{
    static StationManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new StationManager();
    }
    return instance;
}

StationManager::StationManager(QObject *parent)
    : MManagerBase{"stations", parent}
{
    mS1 = new MStation1(this);
    mSmear = new MStationSmear(this);

    addModule(mS1, mSmear);

    m_StationMap.insert(mS1->devid(), mS1);
    m_StationMap.insert(mSmear->devid(), mSmear);

    /* Scan List */
    mScanStationList.append(mS1);
    mScanStationList.append(mSmear);

    /* Test List */
    mTestStationList.append(mS1);
    mTestStationList.append(mSmear);

    mReqTimer = new QTimer(this);
    mReqTimer->setInterval(100);
    connect(mReqTimer, &QTimer::timeout, this, &StationManager::onReqTimer_slot);

    m_stationPriority = StationPriority::First_First;

    state_init();
}

void StationManager::state_init()
{
    m_isReqFinished = true;
    m_TestStationMap.clear();
    m_reqSample = nullptr;
    m_availableStations.clear();
    m_checkStation = nullptr;
    s_request = RequestState::Idle;
}

void StationManager::updateStationStatus()
{
    JPacket packet(PacketType::Notification);
    packet.device = "United";
    packet.module = "Staton";
    packet.api = "StatusChange";
    packet.paramsValue = getStationStatus();
    FMessageCenter::GetInstance()->sendUIMessage(packet);
}

QJsonArray StationManager::getStationStatus()
{
    QJsonArray arr;
    QJsonObject stat1_obj = getStationStatus(mS1);
    QJsonObject stat2_obj = getStationStatus(mSmear);
    arr.append(stat1_obj);
    arr.append(stat2_obj);
    return arr;
}

QJsonObject StationManager::getStationStatus(MStation *station)
{
    QJsonObject obj;
    obj.insert("dev_id", station->devid());
    obj.insert("client_id", station->clientid());
    obj.insert("isConnected", station->isConnected());
    obj.insert("isUnited", station->isUnited());
    return obj;
}

MStation *StationManager::getScanStation()
{
    for (int i = 0; i < mScanStationList.count(); ++i) {
        auto station = mScanStationList.at(i);
        if (station->isConnected() && station->isUnited()) {
            return station;
        }
    }
    return nullptr;
}

QList<MStation *> StationManager::getTestStationList(QSharedPointer<RtSample> sample)
{
    QList<MStation *> list;
    QString program = sample->nextProgram();
    if (program.isEmpty()) {
        return list;
    } else if (program == "smear") {
        /* Smear */
        if (mSmear->isConnected() && mSmear->isUnited()) {
            list.append(mSmear);
        }
    } else {
        /* S1 */
        if (mS1->isConnected() && mS1->isUnited()) {
            if (mS1->programContains(program)) {
                list.append(mS1);
            }
        }
        /* S2 */
        //..

        /* Priority */
        if (list.count() > 1) {
            if (m_stationPriority == StationPriority::Nearest) {
                modifyNearestStationList(list, sample->coord());
            }
        }
    }

    sample->setDoingProgram(program);

    qDebug().noquote() << QString("Sample['%1'] Program:'%2' Station Available List:").arg(sample->sid()).arg(program)
                       << list;
    return list;
}

void StationManager::setStationUnited(const QJsonObject &obj)
{
    if (obj.contains("dev_id")) {
        QString dev_id = obj.value("dev_id").toString();
        if (m_StationMap.contains(dev_id)) {
            MStation *station = m_StationMap.value(dev_id);
            if (station && obj.contains("united")) {
                bool united = obj.value("united").toBool();
                station->setUnited(united);
            }
        }
    }
}

void StationManager::setStationInfo(const QJsonObject &obj)
{
    if (obj.contains("dev_id")) {
        QString dev_id = obj.value("dev_id").toString();
        if (m_StationMap.contains(dev_id)) {
            MStation *station = m_StationMap.value("dev_id");
            if (station) {
                if (obj.contains("666")) {
                    qDebug() << "666" << obj;
                }
            }
        }
    }
}

bool StationManager::reqSendStation(QSharedPointer<RtSample> sample)
{
    if (sample && m_isReqFinished == true) {
        m_isReqFinished = false;
        m_reqSample = sample;
        mReqTimer->start();

        qDebug().noquote() << QString("Sample['%1'] coord:%2 is requiring Send-Station.")
                              .arg(sample->sid()).arg(sample->coord());
        return true;
    }
    return false;
}

bool StationManager::requestSmearStation(QSharedPointer<RtSample> sample)
{
    if (mSmear->isConnected()) {
        if (sample && m_isReqFinished == true) {
            m_isReqFinished = false;
            m_reqSample = sample;
            mReqTimer->start();

            qDebug() << "request Smear Station:" << sample->sid();
            return true;
        }
    }
    return false;
}

MStation *StationManager::getDestinationStation(const QString &sid)
{
    if (m_TestStationMap.contains(sid)) {
        return m_TestStationMap.take(sid);
    }
    return nullptr;
}

void StationManager::onReqTimer_slot()
{
    switch (s_request) {
    case RequestState::Idle:
        if (m_reqSample != nullptr) {
            m_availableStations = getTestStationList(m_reqSample);
            s_request = RequestState::Choose_One_Station;
        }
        break;

    case RequestState::Choose_One_Station:
        if (m_availableStations.isEmpty() == false) {
            m_checkStation = m_availableStations.takeFirst();
            s_request = RequestState::Check_Station;
        } else {
            s_request = RequestState::Finish;
        }
        break;

    case RequestState::Check_Station:
    {
        bool ok = m_checkStation->checkTestAvailable(m_reqSample);
        if (ok) {
            s_request = RequestState::WaitF_Station_Done;
        }
    }
        break;
    case RequestState::WaitF_Station_Done:
        if (m_checkStation->isCheckFinished()) {
            if (m_checkStation->isTestAvailable(m_reqSample->sid()) == true) {
                m_TestStationMap.insert(m_reqSample->sid(), m_checkStation);

                qDebug().noquote() << QString("Station:'%1' ready to pickup sample:'%2'")
                                      .arg(m_checkStation->userid()).arg(m_reqSample->sid());

                s_request = RequestState::Assign_TestUID_From_DMU;
            } else {
                s_request = RequestState::Choose_One_Station;
            }
        }
        break;

    case RequestState::Assign_TestUID_From_DMU:
    {
        bool ok = RtSampleManager::GetInstance()->requestSampleTestUid(m_checkStation->devid(), m_reqSample);
        if (ok) {
            s_request = RequestState::WaitF_Get_TestUID;
        }
    }
        break;

    case RequestState:: WaitF_Get_TestUID:
        if (m_reqSample->test_uid().isEmpty() == false) {
            qDebug() << m_reqSample->sid() << "Get Test_uid:" << m_reqSample->test_uid();
            s_request = RequestState::Finish;
        }
        break;

    case RequestState::Finish:
        m_isReqFinished = true;
        m_reqSample = nullptr;
        mReqTimer->stop();
        s_request = RequestState::Idle;
        break;
    }
}

void StationManager::modifyNearestStationList(QList<MStation *> list, int coord)
{
    if (list.count() == 2 && coord > 0) {
        auto s1 = list.at(0);
        auto s2 = list.at(1);

        int a1 = qAbs(s1->Pos_Test - coord);
        int a2 = qAbs(s2->Pos_Test - coord);
        if (a1 > a2) {
            list.swap(0, 1);
            qDebug() << "change station priority:" << list;
        }
    }
}

MStation *StationManager::getNearestStation(QList<MStation *> stationList, int coord)
{
    if (stationList.isEmpty()) {
        return nullptr;
    } else if (stationList.count() == 1) {
        return stationList.first();
    } else if (stationList.count() == 2) {
        auto s1 = stationList.at(0);
        auto s2 = stationList.at(1);

        int a1 = qAbs(s1->Pos_Test - coord);
        int a2 = qAbs(s2->Pos_Test - coord);
        if (a1 <= a2) {
            return s1;
        } else {
            return s2;
        }
    } else if (stationList.count() == 3) {
        auto s1 = stationList.at(0);
        auto s2 = stationList.at(1);
        auto S2 = stationList.at(2);

        int a1 = qAbs(s1->Pos_Test - coord);
        int a2 = qAbs(s2->Pos_Test - coord);
        int a3 = qAbs(S2->Pos_Test - coord);

        QMap<int, MStation*> map;
        map.insert(a1, s1);
        map.insert(a2, s2);
        map.insert(a3, S2);

        int min = qMin(a1, a2);
        min = qMin(min, a3);
        return map.value(min);
    }
    return nullptr;
}

