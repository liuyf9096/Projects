#ifndef M_RECYCLEBOX_MGR_H
#define M_RECYCLEBOX_MGR_H

#include "module_base.h"
#include "recycle_box.h"

class MRecycleBoxMgr : public DModuleBase
{
    Q_OBJECT
public:
    static MRecycleBoxMgr *GetInstance();

    enum BoxState { Unknown, Available, None };

    void setBoxMaxCapacity(int count);
    void startRun(bool on);
    void setClearMode(bool on);

    void renewBox();
    bool ejectCurrentBox();
    bool isIdle();

    void setHeaterParams(const QJsonObject &obj);
    void setHeatTime(int sec);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    BoxState getBoxState() { return m_boxState; }
    QSharedPointer<RecycleBox> getCurrentBox();

private:
    explicit MRecycleBoxMgr(QObject *parent = nullptr);

    void _setBoxRange();
    void state_init();

    QSharedPointer<RecycleBox> mRecycleBox = nullptr;
    QSharedPointer<RecycleBox> newRecycleBox();
    BoxState m_boxState;

    bool m_startRun;
    bool IsReaderEnable;
    bool IsHeaterEnable;
    bool IsCheckSensorValue;

    bool m_isBoxAvailable;
    bool m_isclearMode;

    QString m_api;

    int mFrom;
    int mTo;

    quint64 m_boxid;
    bool m_isNoBox;
    int mMaxCount;

    QTimer *m_timer;

    enum class FillBoxState {
        Idle,

        Push_New_Box,
        WaitF_Push_New_Box_Done,

        Check_For_Vacant_Box,
        WaitF_Sensor_Res1,

        Check_Box_Count,
        WaitF_Sensor_Res2,

        WaitF_Box_Full,

        Dry_Check_Sensor_Value,
        Move_To_Dry_Pos,
        WaitF_Sensor_Res3,
        WaitF_Move_To_Dry_Pos_Done,

        Finish
    } s_fillBox;

    QSharedPointer<RecycleBox> mDryBox = nullptr;
    bool m_reader_ready;
    bool m_checkReaderFinished;
    int m_waitforReader;
    enum class DryBoxState {
        Idle,

        Start_Heater,
        WaitF_Start_Heater_On_Done,

        WaitF_Dry_Done,

        PushOut_Box,

        WaitF_Sensor_Res,

        Push_To_Manu_Pos,
        WaitF_PushTo_ManuPos_Done,

        Check_Reader,
        PushToReader,
        WaitF_PushToReader_Done,

        Finish
    } s_dryBox;

    void receiveSlidesProcess();

    void dryProcess();

    QList<QSharedPointer<RecycleBox> > mPushTManuList;
    enum class PushToManuState {
        Idle,

        Check_Manu_Sensor,
        WaitF_Sensor_Res,

        Push_To_Manu_Pos,
        WaitF_PushTo_ManuPos_Done,

        Finish
    } s_pTManu;
    void pushToManuProcess();

    int m_heatTime_sec;
    QTimer *m_heatTimer;
    bool m_heating;

    int currentSpareBoxCount();
    int currentRecycleBoxCount();

private slots:
    void onTimer_slot();
    void onReaderMessageResult_slot(const JPacket &result, const JPacket &request);
};

#endif // M_RECYCLEBOX_MGR_H
