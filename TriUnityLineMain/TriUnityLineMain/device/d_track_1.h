#ifndef D_TRACK_1_H
#define D_TRACK_1_H

#include "rt_device_base.h"

class DTrack1 : public RtDeviceBase
{
    Q_OBJECT
public:
    explicit DTrack1(const QString &dev_id, QObject *parent = nullptr);
    virtual ~DTrack1() {}

    bool cmd_Import_Load();
    bool cmd_Import_Reset();

    bool cmd_Exit_E1_Unload();
    bool cmd_Exit_E1_Reset();

    bool cmd_S1_Emergency_Open();
    bool cmd_S1_Emergency_Close();

    bool cmd_S1_Scan_Open(int timeout = 2500);
    bool cmd_S1_Scan_Close();
};

#endif // D_TRACK_1_H
