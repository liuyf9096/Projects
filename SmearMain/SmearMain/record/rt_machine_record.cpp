#include "rt_machine_record.h"
#include "f_common.h"

#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>

//#define UsingBinaryData

RtMachineRecord *RtMachineRecord::GetInstance()
{
    static RtMachineRecord *instance = nullptr;
    if (instance == nullptr) {
        instance = new RtMachineRecord();
    }
    return instance;
}

RtMachineRecord::RtMachineRecord(QObject *parent) : QObject(parent)
{
    QDir config_dir(FCommon::getPath("config"));
    mFilePath = config_dir.absoluteFilePath("record.json");
    qDebug() << "record file path:" << mFilePath;
}

void RtMachineRecord::read()
{
    QFile file(mFilePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "read record, file open fail.";
        return;
    }

    QByteArray data = file.readAll();

#ifdef UsingBinaryData
    QJsonDocument doc(QJsonDocument::fromBinaryData(data));
#else
    QJsonDocument doc(QJsonDocument::fromJson(data));
#endif

    mObj = doc.object();

    mTrackObj = mObj.value("track").toObject();
    mSamplingObj = mObj.value("sampling").toObject();
    mSmearObj = mObj.value("smear").toObject();
    mStainObj = mObj.value("stain").toObject();
    mAgingTestObj = mObj.value("agingTest").toObject();
    agingStain = mAgingTestObj.value("stain").toInt();
    agingSmear = mAgingTestObj.value("smear").toInt();

    /* track */
    isScanning = mTrackObj.value("isScanning").toBool();

    /* sampling */
    currentSample   = mSamplingObj.value("currentSample").toString();
    isGrip          = mSamplingObj.value("isGrip").toBool();
    isEmergency     = mSamplingObj.value("isEmergency").toBool();
    rack_id         = mSamplingObj.value("rack_id").toString();
    rack_pos        = mSamplingObj.value("rack_pos").toInt();

    /* stain */
    QJsonObject pools_obj = mStainObj.value("pools").toObject();
    QJsonObject fix_obj = pools_obj.value("fix").toObject();
    QJsonObject a1_obj  = pools_obj.value("a1").toObject();
    QJsonObject c1_obj  = pools_obj.value("c1").toObject();
    QJsonObject c2a_obj = pools_obj.value("c2a").toObject();
    QJsonObject c2b_obj = pools_obj.value("c2b").toObject();

    pools.fix.isFilled  = fix_obj.value("isFilled").toBool();
    pools.fix.count     = fix_obj.value("count").toInt();
    pools.fix.timeHr    = fix_obj.value("timeHr").toInt();

    pools.a1.isFilled   = a1_obj.value("isFilled").toBool();
    pools.a1.count      = a1_obj.value("count").toInt();
    pools.a1.timeHr     = a1_obj.value("timeHr").toInt();

    pools.c1.isFilled   = c1_obj.value("isFilled").toBool();
    pools.c1.count      = c1_obj.value("count").toInt();
    pools.c1.timeHr     = c1_obj.value("timeHr").toInt();

    pools.c2a.isFilled  = c2a_obj.value("isFilled").toBool();
    pools.c2a.count     = c2a_obj.value("count").toInt();
    pools.c2a.timeHr    = c2a_obj.value("timeHr").toInt();

    pools.c2b.isFilled  = c2b_obj.value("isFilled").toBool();
    pools.c2b.count     = c2b_obj.value("count").toInt();
    pools.c2b.timeHr    = c2b_obj.value("timeHr").toInt();

    qDebug() << "record content:" << mObj;
}

void RtMachineRecord::_write()
{
    QFile file(mFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "write record, file open fail.";
        return;
    }

    QJsonDocument doc(mObj);
#ifdef UsingBinaryData
    file.write(doc.toBinaryData());
#else
    file.write(doc.toJson());
#endif
}

void RtMachineRecord::write()
{
    /* track */
    mTrackObj.insert("isScanning", isScanning);

    /* sampling */
    mSamplingObj.insert("currentSample", currentSample);
    mSamplingObj.insert("isGrip", isGrip);
    mSamplingObj.insert("isEmergency", isEmergency);
    mSamplingObj.insert("rack_id", rack_id);
    mSamplingObj.insert("rack_pos", rack_pos);

    /* stain */
    QJsonObject fixObj {
        {"isFilled", pools.fix.isFilled},
        {"count", pools.fix.count},
        {"timeHr", pools.fix.timeHr}
    };
    QJsonObject a1Obj {
        {"isFilled", pools.a1.isFilled},
        {"count", pools.a1.count},
        {"timeHr", pools.a1.timeHr}
    };
    QJsonObject c1Obj {
        {"isFilled", pools.c1.isFilled},
        {"count", pools.c1.count},
        {"timeHr", pools.c1.timeHr}
    };
    QJsonObject c2aObj {
        {"isFilled", pools.c2a.isFilled},
        {"count", pools.c2a.count},
        {"timeHr", pools.c2a.timeHr}
    };
    QJsonObject c2bObj {
        {"isFilled", pools.c2b.isFilled},
        {"count", pools.c2b.count},
        {"timeHr", pools.c2b.timeHr}
    };
    QJsonObject poolsObj {
        {"fix", fixObj},
        {"a1", a1Obj},
        {"c1", c1Obj},
        {"c2a", c2aObj},
        {"c2b", c2bObj}
    };

    mStainObj.insert("pools", poolsObj);

    mAgingTestObj.insert("stain", agingStain);
    mAgingTestObj.insert("smear", agingSmear);

    mObj.insert("track", mTrackObj);
    mObj.insert("sampling", mSamplingObj);
    mObj.insert("smear", mSmearObj);
    mObj.insert("stain", mStainObj);
    mObj.insert("agingTest", mAgingTestObj);
    mObj.insert("logPath", logPath);

    _write();
}

void RtMachineRecord::writeLogPath(const QString &path)
{
    logPath = path;
    mObj.insert("logPath", path);
    _write();
}

void RtMachineRecord::clearSampling()
{
    currentSample.clear();
    isGrip = false;
    isEmergency = false;
    rack_id.clear();
    rack_pos = -1;
}
