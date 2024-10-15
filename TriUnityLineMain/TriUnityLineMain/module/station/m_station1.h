#ifndef M_STATION1_H
#define M_STATION1_H

#include "m_station.h"

class MStation1 : public MStation
{
    Q_OBJECT
public:
    explicit MStation1(QObject *parent = nullptr);

    virtual void setUnited(bool isUnited) override;
    virtual void setClientId(const QString &clientid) override;

    virtual void cmd_Scan_Open() override;
    virtual void cmd_Scan_Close() override;
    virtual void cmd_Emergency_Open() override;
    virtual void cmd_Emergency_Close() override;
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

#endif // M_STATION1_H
