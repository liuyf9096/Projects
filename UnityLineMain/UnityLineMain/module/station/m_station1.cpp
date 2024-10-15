#include "m_station1.h"
#include "f_common.h"

MStation1::MStation1(QObject *parent): MStation{"station1", "S1", 10, 12, parent}
{
    auto arr = FCommon::GetInstance()->getConfigValue("line_device").toArray();
    if (arr.count() > 0) {
        QJsonObject obj = arr.first().toObject();
        mDevid = obj.value("devid").toString();
        m_isUnited = obj.value("isUnited").toBool();
    }

    ProgramList << "5-classify" << "crp";
    qDebug() << QString("Station1 [%1] program:").arg(mDevid) << ProgramList
             << "isUnited:" << m_isUnited;

    connect(RtDeviceManager::GetInstance()->track(), &DTrack::onFunctionFinished_signal,
            this, &MStation1::onFunctionFinished_slot);
}

void MStation1::cmd_Scan_Open(int timeout)
{
    dev->track()->cmd_S1_Scan_Open(timeout);
}

void MStation1::cmd_Scan_Close()
{
    dev->track()->cmd_S1_Scan_Close();
}

bool MStation1::cmd_Emergency_Open()
{
    return dev->track()->cmd_S1_Emergency_Open();
}

bool MStation1::cmd_Emergency_Close()
{
    return dev->track()->cmd_S1_Emergency_Close();
}

void MStation1::cmd_CheckSensorValue()
{
    dev->track()->cmd_CheckSensorValue();
}

bool MStation1::isScanOpen_Done()
{
    return dev->track()->isFuncDone("S1_Scan_Open");
}

bool MStation1::isScanClose_Done()
{
    return dev->track()->isFuncDone("S1_Scan_Close");
}

bool MStation1::isEmergencyOpen_Done()
{
    return dev->track()->isFuncDone("S1_Emergency_Open");
}

bool MStation1::isEmergencyClose_Done()
{
    return dev->track()->isFuncDone("S1_Emergency_Close");
}

bool MStation1::isCheckSensorValue_Done()
{
    return dev->track()->isFuncDone("CheckSensorValue");
}

bool MStation1::isSampleExist()
{
    return dev->track()->checkSensorValue("S1_scan_sample_exist");
}

void MStation1::receiveFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Send_To_Test, 1);
    sample->setState(SampleStatus::SamplingFinished, 0);
    sample->setState(SampleStatus::Recycle_To_Rack, 0);
    sample->removeDoingProgram();
}

void MStation1::recycleFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Recycle_To_Rack, 1);
    if (sample->nextProgram().isEmpty()) {
        sample->setState(SampleStatus::Test_Finished, 1);
    }
}

void MStation1::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api.startsWith("S1") == false) {
        return;
    }

    if (api == "S1_Scan_Open") {
        mScanBarcode.clear();
        if (resValue.isObject()) {
            QJsonObject obj = resValue.toObject();
            mScanBarcode = obj.value("string").toString().simplified();
        }
    }
}
