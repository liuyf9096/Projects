#include "station_manager.h"
#include "m_station1.h"
#include "m_station2.h"
#include "m_station3.h"
#include "settings/f_settings.h"
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
    mS2 = new MStation2(this);
    mS3 = new MStation3(this);

    addModule(mS1, mS2, mS3);

    m_StationMap.insert(mS1->devid(), mS1);
    m_StationMap.insert(mS2->devid(), mS2);
    m_StationMap.insert(mS3->devid(), mS3);

    mScanStationList.append(mS1);
    mScanStationList.append(mS2);
    mScanStationList.append(mS3);

    mTestStationList.append(mS1);
    mTestStationList.append(mS2);
    mTestStationList.append(mS3);

    m_reqTimer = new QTimer(this);
    m_reqTimer->setInterval(80);
    connect(m_reqTimer, &QTimer::timeout, this, &StationManager::onReqTimer_slot);

    m_isRequestFinished = true;
    s_request = RequestState::Idle;
}

void StationManager::updateStationStatus()
{
    JPacket packet(PacketType::Notification);
    packet.device = QStringLiteral("United");
    packet.module = QStringLiteral("Staton");
    packet.api = QStringLiteral("StatusChange");
    QJsonArray arr = StationManager::GetInstance()->getStationStatus();
    packet.paramsValue = arr;
    FMessageCenter::GetInstance()->sendUIMessage(packet);
}

QJsonArray StationManager::getStationStatus()
{
    QJsonObject stat1_obj;
    stat1_obj.insert("dev_id", mS1->devid());
    stat1_obj.insert("client_id", mS1->clientid());
    stat1_obj.insert("isConnected", mS1->isConnected());
    stat1_obj.insert("isUnited", mS1->isUnited());

    QJsonObject stat2_obj;
    stat2_obj.insert("dev_id", mS2->devid());
    stat2_obj.insert("client_id", mS2->clientid());
    stat2_obj.insert("isConnected", mS2->isConnected());
    stat2_obj.insert("isUnited", mS2->isUnited());

    QJsonObject stat3_obj;
    stat3_obj.insert("dev_id", mS3->devid());
    stat3_obj.insert("client_id", mS3->clientid());
    stat3_obj.insert("isConnected", mS3->isConnected());
    stat3_obj.insert("isUnited", mS3->isUnited());

    QJsonArray arr;
    arr.append(stat1_obj);
    arr.append(stat2_obj);
    arr.append(stat3_obj);
    return arr;
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

QList<MStation *> StationManager::getTestStationList(const QString &program)
{
    QList<MStation*> list;
    for (int i = 0; i < mTestStationList.count(); ++i) {
        auto station = mTestStationList.at(i);
        if (station->isConnected() && station->isUnited()) {
            if (station->programContains(program)) {
                list.append(station);
            }
        }
    }
    qDebug().noquote() << QString("Program:%1 Station Available:").arg(program) << list;
    return list;
}

void StationManager::setStationUnited(const QJsonObject &obj)
{
    if (obj.contains("dev_id")) {
        QString dev_id = obj.value("dev_id").toString();
        if (m_StationMap.contains(dev_id)) {
            MStation *station = m_StationMap.value(dev_id);
            if (station) {
                if (obj.contains("united")) {
                    bool united = obj.value("united").toBool();
                    station->setUnited(united);
                }
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

bool StationManager::isSmearOnly()
{
    if (mS3->isConnected() && mS3->isUnited()) {
        if (mS1->isUnited() == false && mS2->isUnited() == false) {
            return true;
        }
    }
    return false;
}

bool StationManager::requestTestStation(QSharedPointer<RtSample> sample, int coord)
{
    if (sample && m_isRequestFinished == true) {
        m_isRequestFinished = false;
        m_isSmear = false;
        m_reqSample = sample;
        mReqCoord = coord;
        m_reqTimer->start();
        qDebug() << "request Test Station:" << sample->sid() << coord;
        return true;
    }
    return false;
}

bool StationManager::requestSmearStation(QSharedPointer<RtSample> sample)
{
    if (mS3->isConnected()) {
        if (sample && m_isRequestFinished == true) {
            m_isRequestFinished = false;
            m_isSmear = true;
            m_reqSample = sample;
            m_reqTimer->start();
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
            if (m_isSmear == false) {
                qDebug() << "StationManager 1 Check_Nearest_Station";
                s_request = RequestState::Check_Nearest_Station;
            } else {
                qDebug() << "StationManager 1 Check_Smear_Station";
                s_request = RequestState::Check_Smear_Station;
            }
        }
        break;

    case RequestState::Check_Nearest_Station:
    {
        auto stationList = getTestStationList(m_reqSample->program());
        auto station = getNearestStation(stationList, mReqCoord);
        if (station) {
            mNearestStaion = station;
            bool ok = mNearestStaion->checkTestAvailable(m_reqSample);
            if (ok) {
                s_request = RequestState::WaitF_Nearest_Done;
            }
        }
    }
        break;
    case RequestState::WaitF_Nearest_Done:
        if (mNearestStaion->isCheckFinished()) {
            if (mNearestStaion->isTestAvailable(m_reqSample->sid()) == true) {
                m_TestStationMap.insert(m_reqSample->sid(), mNearestStaion);
                qDebug().noquote() << QString("Station Rquest Station:%1 sample:%2")
                                      .arg(mNearestStaion->userid()).arg(m_reqSample->sid());
                s_request = RequestState::Finish;
            } else {
                auto stationList = getTestStationList(m_reqSample->program());
                if (stationList.count() > 1) {
                    s_request = RequestState::Check_Other;
                } else {
                    s_request = RequestState::Finish;
                }
            }
        }
        break;

    case RequestState::Check_Other:
    {
        qDebug() << "Check Other Station";
        auto stationList = getTestStationList(m_reqSample->program());
        stationList.removeOne(mNearestStaion);

        mOtherStaion = stationList.first();
        mOtherStaion->checkTestAvailable(m_reqSample);
        s_request = RequestState::WaitF_Other_Done;
    }
        break;
    case RequestState::WaitF_Other_Done:
        if (mOtherStaion->isCheckFinished()) {
            if (mOtherStaion->isTestAvailable(m_reqSample->sid())) {
                m_TestStationMap.insert(m_reqSample->sid(), mOtherStaion);
            }
            s_request = RequestState::Finish;
        }
        break;

    case RequestState::Check_Smear_Station:
    {
        mS3->checkTestAvailable(m_reqSample);
        s_request = RequestState::WaitF_Smear_Done;
    }
        break;
    case RequestState::WaitF_Smear_Done:
        if (mS3->isCheckFinished()) {
            if (mS3->isTestAvailable(m_reqSample->sid())) {
                m_TestStationMap.insert(m_reqSample->sid(), mS3);
            }
            s_request = RequestState::Finish;
        }
        break;

    case RequestState::Finish:
        qDebug() << "StationManager 99 RequestState Finish";
        m_isRequestFinished = true;
        m_reqSample = nullptr;
        m_reqTimer->stop();
        s_request = RequestState::Idle;
        break;
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
        auto s3 = stationList.at(2);

        int a1 = qAbs(s1->Pos_Test - coord);
        int a2 = qAbs(s2->Pos_Test - coord);
        int a3 = qAbs(s3->Pos_Test - coord);

        QMap<int, MStation*> map;
        map.insert(a1, s1);
        map.insert(a2, s2);
        map.insert(a3, s3);

        int min = qMin(a1, a2);
        min = qMin(min, a3);
        return map.value(min);
    }
    return nullptr;
}

