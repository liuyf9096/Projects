#ifndef M_STATION_H
#define M_STATION_H

#include "module_base.h"
#include "sample/rt_sample.h"

#include <QList>

class RtRack;
class MCart;
class MStation : public DModuleBase
{
    Q_OBJECT
public:
    explicit MStation(const QString &mid, const QString &userid, int posscan, int postest, QObject *parent = nullptr);

    const int Pos_Scan;
    const int Pos_Test;

    virtual void setClientId(const QString &clientid);
    bool isConnected() { return m_isConnected; }
    QString devid() { return mClientDevid; }
    QString clientid() { return mClientid; }

    virtual void setUnited(bool isUnited) { m_isUnited = isUnited; }
    bool isUnited() { return m_isUnited; }

    bool scanSampleBarcode(QSharedPointer<RtRack> rack, int pos);
    bool isScanFinished() { return m_isScanFinished; }

    bool checkTestAvailable(QSharedPointer<RtSample> sample);
    bool isCheckFinished() { return m_isStaionCheckIdleFinished; }
    bool isTestAvailable(const QString &sid);

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    /* 2.receive sample */
    bool receiveNewSample(QSharedPointer<RtSample> sample);
    bool receiveSampleFinish() { return m_isReceiveFinished; }

    /* 3.recycle sample */
    bool recycleSample(QSharedPointer<RtSample> sample);
    bool isRecycleSampleFinish() { return m_isRecycleFinished; }

    /* supported programs */
    void setSupportedPrograms(const QStringList &list) { ProgramList = list; }
    bool programContains(const QString &program) { return ProgramList.contains(program); }

protected:
    QString mClientDevid;
    QString mClientid;
    bool m_isConnected;
    bool m_isUnited;

    QString mScanBarcode;
    QStringList ProgramList;

    quint64 closetOpen_id;
    quint64 closetClose_id;

    virtual void cmd_Scan_Open() = 0;
    virtual void cmd_Scan_Close() = 0;
    virtual void cmd_Emergency_Open() = 0;
    virtual void cmd_Emergency_Close() = 0;
    virtual void cmd_CheckSensorValue() = 0;

    virtual bool isScanOpen_Done() = 0;
    virtual bool isScanClose_Done() = 0;
    virtual bool isEmergencyOpen_Done() = 0;
    virtual bool isEmergencyClose_Done() = 0;
    virtual bool isCheckSensorValue_Done() = 0;
    virtual bool isSampleExist() = 0;

    virtual void receiveFinished(QSharedPointer<RtSample> sample);
    virtual void recycleFinished(QSharedPointer<RtSample> sample);

    void updateStationStatus();

protected slots:
    virtual void onFunctionFinished_slot(const QString &api, const QJsonValue &resValue) = 0;

private:
    QMap<QString, bool> m_checkAvailableMap;
    QMap<quint64, QString> m_queryMap;

    void _timer_init();
    void state_init();

    /* Scan */
    bool m_isScanFinished;
    int mScanPos;
    QSharedPointer<RtRack> mScanRack;
    QSharedPointer<RtSample> mScanSample;
    QTimer *m_ScanTimer;
    enum class ScanState {
        Idle,

        Check_Sensor,
        WaitF_Sensor_Value,
        Check_Tube_Exist,

        Scan_Tube,
        WaitF_Rotate_Tube_Finished,

        Finish
    } s_Scan;

    /* receive sample */
    bool m_isReceiveFinished;
    QSharedPointer<RtSample> m_receiveSample;
    QTimer *m_ReceiveTimer;
    enum class ReceiveState {
        Idle,

        Send_Receive_Sample_Cmd,
        WaitF_Receive_Sample_Cmd_Done,

        Finish
    } s_Receive;

    /* recycle sample */
    bool m_isRecycleFinished;
    QSharedPointer<RtSample> m_recycleSample;
    QTimer *m_RecycleTimer;
    enum class RecycleState {
        Idle,

        Send_Recycle_Sample_Cmd,
        WaitF_Recycle_Sample_Cmd_Done,

        Finish
    } s_Recycle;

    bool m_isStaionCheckIdleFinished;
    void sendStationCheckIdle(QSharedPointer<RtSample> sample);
    bool m_isStationRotateTubeFinished;
    bool m_isStationRotateTubeError;
    void sendStationRotateTube();
    bool m_isStationTakeUpNewSampleFinished;
    void sendStationTakeUpNewSample(QSharedPointer<RtSample> sample);
    bool m_isStationRecycleSampleFinished;
    void sendStationRecycleSampleToRack(QSharedPointer<RtSample> sample);

private slots:
    void handleConnect_slot(const QString &client_dev, const QString &client_id, bool connected);
    void handleRequestMessage_slot(const QString &client_id, const JPacket &request);
    void handleResultMessage_slot(const QString &client_id, const JPacket &result, const JPacket &request);
    void handleErrorMessage_slot(const QString &client_id, const JPacket &error, const JPacket &request);

    void onScanTimer_slot();
    void onReceiveTimer_slot();
    void onRecycleTimer_slot();
};

#endif // M_STATION_H
