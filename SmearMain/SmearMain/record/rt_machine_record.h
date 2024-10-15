#ifndef RT_MACHINE_RECORD_H
#define RT_MACHINE_RECORD_H

#include <QObject>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

struct PoolInfo
{
public:
    PoolInfo():count(0), timeHr(0) {}

    bool isFilled;
    int count;
    int timeHr;
};

struct Pools
{
public:
    Pools() {}

    PoolInfo fix;
    PoolInfo a1;
    PoolInfo c1;
    PoolInfo c2a;
    PoolInfo c2b;
};

class RtMachineRecord : public QObject
{
    Q_OBJECT
public:
    static RtMachineRecord *GetInstance();

    void read();
    void write();

    void writeLogPath(const QString &path);

    /* sampling */
    QString currentSample;
    bool isGrip;
    bool isEmergency;
    QString rack_id;
    int rack_pos;
    void clearSampling();

    /* stain */
    Pools pools;

    QJsonObject mObj;

    QJsonObject mTrackObj;
    QJsonObject mSamplingObj;
    QJsonObject mSmearObj;
    QJsonObject mStainObj;
    QJsonObject mAgingTestObj;

    int agingStain;
    int agingSmear;
    QString logPath;
    bool isScanning;

private:
    explicit RtMachineRecord(QObject *parent = nullptr);
    Q_DISABLE_COPY(RtMachineRecord)

    QString mFilePath;
    void _write();
};

#endif // RT_MACHINE_RECORD_H
