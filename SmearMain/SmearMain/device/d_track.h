#ifndef D_TRACK_H
#define D_TRACK_H

#include "rt_device_base.h"

class DTrack : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DTrack(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DTrack() {}

    bool cmd_C1_Reset();
    bool cmd_C1_Pos_Import();
    bool cmd_C1_Pos_Scan(int pos);
    bool cmd_C1_Pos_Test(int pos);
    bool cmd_C1_Pos_Export();
    bool cmd_C1_Pos_Left(int pos);
    bool cmd_C1_Pos_Right(int pos);

    bool cmd_C2_Reset();
    bool cmd_C2_Pos_Import();
    bool cmd_C2_Pos_Scan(int pos);
    bool cmd_C2_Pos_Test(int pos);
    bool cmd_C2_Pos_Export();
    bool cmd_C2_Pos_Left(int pos);
    bool cmd_C2_Pos_Right(int pos);

    bool cmd_Import_Reset();
    bool cmd_Import_Load();

    bool cmd_Export_Reset();
    bool cmd_Export_Unload();

    bool cmd_Emergency_Open();
    bool cmd_Emergency_Close();

    bool cmd_Scan_Open(int timeout);
    bool cmd_Scan_Close();

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue) override;
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj) override;

private:
    bool m_Load_Lock;
};

#endif // D_TRACK_H

