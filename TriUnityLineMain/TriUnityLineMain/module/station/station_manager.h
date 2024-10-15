#ifndef STATION_MANAGER_H
#define STATION_MANAGER_H

#include "m_manager_base.h"
#include "m_station.h"

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

    MStation *getScanStation();
    QList<MStation*> getTestStationList(const QString &program);

    void setStationUnited(const QJsonObject &obj);
    void setStationInfo(const QJsonObject &obj);

    bool isSmearOnly();
    bool requestTestStation(QSharedPointer<RtSample> sample, int coord);
    bool requestSmearStation(QSharedPointer<RtSample> sample);
    bool isRequestTestStationFinished() { return m_isRequestFinished; }
    MStation *getDestinationStation(const QString &sid);

    MStation *mS1;
    MStation *mS2;
    MStation *mS3;

private:
    explicit StationManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(StationManager)

    QMap<QString, MStation*> m_StationMap;
    QList<MStation*> mScanStationList;
    QList<MStation*> mTestStationList;

    QMap<QString, MStation*> m_TestStationMap;

    bool m_isRequestFinished;
    bool m_isSmear;
    QSharedPointer<RtSample> m_reqSample;
    int mReqCoord;
    QTimer *m_reqTimer;
    MStation *mNearestStaion;
    MStation *mOtherStaion;
    enum class RequestState {
        Idle,

        Check_Nearest_Station,
        WaitF_Nearest_Done,

        Check_Other,
        WaitF_Other_Done,

        Check_Smear_Station,
        WaitF_Smear_Done,

        Finish
    } s_request;

    MStation *getNearestStation(QList<MStation*> stationList, int coord);

private slots:
    void onReqTimer_slot();
};

#endif // STATION_MANAGER_H
