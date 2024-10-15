#ifndef D_TRACK_2_H
#define D_TRACK_2_H

#include "rt_device_base.h"

class DTrack2 : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DTrack2(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DTrack2() {}

    bool cmd_S3_Emergency_Close();
    bool cmd_S3_Emergency_Open();

    bool cmd_S3_Scan_Open(int timeout = 1500);
    bool cmd_S3_Scan_Close();

    bool cmd_Rotate_Start();
};

#endif // D_TRACK_2_H
