#ifndef M_CHECK_SENSORS_PROCESS_H
#define M_CHECK_SENSORS_PROCESS_H

#include "m_process_base.h"

class MCheckSensorsProcess : public MProcessBase
{
    Q_OBJECT
public:
    explicit MCheckSensorsProcess(QObject *parent = nullptr);

    virtual bool startProcess() override;
    void checkOnce();
    void startCheck();

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();
    void _getFloaterId();

    QMap<int, QString> m_SampleFloaterIdMap;
    QMap<int, QString> m_StainFloaterIdMap;
    QMap<QString, int> m_FloaterAlarmValueMap;
    QStringList m_errorList;
    QStringList m_sendAlarmList;

    QStringList m_sampleErrorList;
    QStringList m_stainErrorList;
    QStringList m_sendSampleAlarmList;
    QStringList m_sendStainAlarmList;

    QTimer *m_timer;
    enum class FloaterState {
        Check_Floater1,
        Check_Floater2,
        WaitF_Check_FloatSensor,
        Send_Alarm,
        Finish
    } s_floater;

    void sendAlarm();
    void clearAlarm();

    void checkFloaterProc();

private slots:
    void handleSampleFunctionFinish_slot(const QString &api, const QJsonValue &resValue);
    void handleStainFunctionFinish_slot(const QString &api, const QJsonValue &resValue);
    void handleSampleFloaterSensorNote_slot(const QString &dev_id, const QJsonObject &obj);
    void handleStainFloaterSensorNote_slot(const QString &dev_id, const QJsonObject &obj);
};

#endif // M_CHECK_SENSORS_PROCESS_H
