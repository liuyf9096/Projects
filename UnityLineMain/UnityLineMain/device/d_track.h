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
    bool cmd_C1_Pos_S1_Scan(int pos);
    bool cmd_C1_Pos_S1_Test(int pos);
    bool cmd_C1_Pos_S2_Scan(int pos);
    bool cmd_C1_Pos_S2_Test(int pos);
    bool cmd_C1_Pos_S3_Scan(int pos);
    bool cmd_C1_Pos_S3_Test(int pos);
    bool cmd_C1_Pos_Exit_E1();
    bool cmd_C1_Pos_Exit_E2();
    bool cmd_C1_Pos_Left(int pos);
    bool cmd_C1_Pos_Right(int pos);
    bool cmd_C1_Pos_S1_RackScan();
    bool cmd_C1_Pos_S2_RackScan();
    bool cmd_C1_Pos_S3_RackScan();

    bool cmd_C2_Reset();
    bool cmd_C2_Pos_Import();
    bool cmd_C2_Pos_S1_Scan(int pos);
    bool cmd_C2_Pos_S1_Test(int pos);
    bool cmd_C2_Pos_S2_Scan(int pos);
    bool cmd_C2_Pos_S2_Test(int pos);
    bool cmd_C2_Pos_S3_Scan(int pos);
    bool cmd_C2_Pos_S3_Test(int pos);
    bool cmd_C2_Pos_Exit_E1();
    bool cmd_C2_Pos_Exit_E2();
    bool cmd_C2_Pos_Left(int pos);
    bool cmd_C2_Pos_Right(int pos);
    bool cmd_C2_Pos_S1_RackScan();
    bool cmd_C2_Pos_S2_RackScan();
    bool cmd_C2_Pos_S3_RackScan();

    bool cmd_Import_Load();
    bool cmd_Import_Reset();

    bool cmd_Exit_E1_Unload();
    bool cmd_Exit_E1_Reset();

    bool cmd_Exit_E2_Unload();
    bool cmd_Exit_E2_Reset();

    bool cmd_S1_Emergency_Open();
    bool cmd_S1_Emergency_Close();

    bool cmd_S2_Emergency_Open();
    bool cmd_S2_Emergency_Close();

    bool cmd_S3_Emergency_Open();
    bool cmd_S3_Emergency_Close();

    bool cmd_S1_Scan_Open(int timeout = 2500);
    bool cmd_S1_Scan_Close();

    bool cmd_S2_Scan_Open(int timeout = 2500);
    bool cmd_S2_Scan_Close();

    bool cmd_S3_Scan_Open(int timeout = 2500);
    bool cmd_S3_Scan_Close();

    bool cmd_Carts_Reset();

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue) override;
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj) override;

private:
    bool m_Closet1_Lock;
    bool m_Closet2_Lock;
    bool m_Closet3_Lock;
};

#endif // D_TRACK_H
