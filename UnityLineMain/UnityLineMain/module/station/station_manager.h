#ifndef STATION_MANAGER_H
#define STATION_MANAGER_H

#include "m_manager_base.h"
#include "m_station.h"
#include "m_station1.h"
#include "m_station_smear.h"

#include <QObject>
#include <QMap>
#include <QList>

class StationManager : public MManagerBase
{
    Q_OBJECT
public:
    static StationManager *GetInstance();

    void addScanStation(MStation *station) { mScanStationList.append(station); }
    void addTestStation(MStation *station) { mTestStationList.append(station); }

    void updateStationStatus();
    QJsonArray getStationStatus();
    QJsonObject getStationStatus(MStation *station);

    MStation *getScanStation();
    QList<MStation*> getTestStationList(QSharedPointer<RtSample> sample);

    void setStationUnited(const QJsonObject &obj);
    void setStationInfo(const QJsonObject &obj);

    bool reqSendStation(QSharedPointer<RtSample> sample);
    bool requestSmearStation(QSharedPointer<RtSample> sample);
    bool isRequestTestStationFinished() { return m_isReqFinished; }
    MStation *getDestinationStation(const QString &sid);

    MStation1 *mS1;
    MStationSmear *mSmear;

    void state_init();

private:
    explicit StationManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(StationManager)

    QMap<QString, MStation*> m_StationMap;
    QList<MStation*> mScanStationList;
    QList<MStation*> mTestStationList;

    QMap<QString, MStation*> m_TestStationMap;

    QTimer *mReqTimer;
    QSharedPointer<RtSample> m_reqSample;
    bool m_isReqFinished;

    QList<MStation *> m_availableStations;
    MStation *m_checkStation;

    enum class StationPriority {
        First_First,
        Nearest
    } m_stationPriority;

    enum class RequestState {
        Idle,

        Choose_One_Station,
        Check_Station,
        WaitF_Station_Done,

        Assign_TestUID_From_DMU,
        WaitF_Get_TestUID,

        Finish
    } s_request;

    void modifyNearestStationList(QList<MStation*> list, int coord);
    MStation *getNearestStation(QList<MStation*> stationList, int coord);

private slots:
    void onReqTimer_slot();
};

#endif // STATION_MANAGER_H
