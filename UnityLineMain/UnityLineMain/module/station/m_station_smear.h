#ifndef M_STATION_SMEAR_H
#define M_STATION_SMEAR_H

#include "m_station.h"

class MStationSmear : public MStation
{
    Q_OBJECT
public:
    explicit MStationSmear(QObject *parent = nullptr);

    virtual void cmd_Scan_Open(int timeout) override;
    virtual void cmd_Scan_Close() override;
    virtual bool cmd_Emergency_Open() override;
    virtual bool cmd_Emergency_Close() override;
    virtual void cmd_CheckSensorValue() override;

    virtual bool isScanOpen_Done() override;
    virtual bool isScanClose_Done() override;
    virtual bool isEmergencyOpen_Done() override;
    virtual bool isEmergencyClose_Done() override;
    virtual bool isCheckSensorValue_Done() override;
    virtual bool isSampleExist() override;

protected:
    virtual void receiveFinished(QSharedPointer<RtSample> sample) override;
    virtual void recycleFinished(QSharedPointer<RtSample> sample) override;
    virtual void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue) override;
};

#endif // M_STATION_SMEAR_H
