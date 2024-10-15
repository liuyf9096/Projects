#include "m_station_smear.h"
#include "f_common.h"

MStationSmear::MStationSmear(QObject *parent): MStation{"stationsmear", "S2", 29, 31, parent}
{
    auto arr = FCommon::GetInstance()->getConfigValue("line_device").toArray();
    if (arr.count() > 1) {
        QJsonObject obj = arr.last().toObject();
        mDevid = obj.value("devid").toString();
        m_isUnited = obj.value("isUnited").toBool();
    }

    ProgramList << "smear";
    qDebug() << QString("StationSmear [%1] program:").arg(mDevid) << ProgramList
             << "isUnited:" << m_isUnited;

    connect(RtDeviceManager::GetInstance()->track(), &DTrack::onFunctionFinished_signal,
            this, &MStationSmear::onFunctionFinished_slot);
}

void MStationSmear::cmd_Scan_Open(int timeout)
{
    dev->track()->cmd_S2_Scan_Open(timeout);
}

void MStationSmear::cmd_Scan_Close()
{
    dev->track()->cmd_S2_Scan_Close();
}

bool MStationSmear::cmd_Emergency_Open()
{
    return dev->track()->cmd_S2_Emergency_Open();
}

bool MStationSmear::cmd_Emergency_Close()
{
    return dev->track()->cmd_S2_Emergency_Close();
}

void MStationSmear::cmd_CheckSensorValue()
{
    dev->track()->cmd_CheckSensorValue();
}

bool MStationSmear::isScanOpen_Done()
{
    return dev->track()->isFuncDone("S2_Scan_Open");
}

bool MStationSmear::isScanClose_Done()
{
    return dev->track()->isFuncDone("S2_Scan_Close");
}

bool MStationSmear::isEmergencyOpen_Done()
{
    return dev->track()->isFuncDone("S2_Emergency_Open");
}

bool MStationSmear::isEmergencyClose_Done()
{
    return dev->track()->isFuncDone("S2_Emergency_Close");
}

bool MStationSmear::isCheckSensorValue_Done()
{
    return dev->track()->isFuncDone("CheckSensorValue");
}

bool MStationSmear::isSampleExist()
{
    return dev->track()->checkSensorValue("S2_scan_sample_exist");
}

void MStationSmear::receiveFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Send_To_Test, 1);
    sample->setState(SampleStatus::SamplingFinished, 0);
    sample->setState(SampleStatus::Recycle_To_Rack, 0);
    sample->setState(SampleStatus::Send_To_Smear, 1);
    sample->removeDoingProgram();
    sample->setState(SampleStatus::Review_Done, 0);
}

void MStationSmear::recycleFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Smear_Done, 1);
    sample->setState(SampleStatus::Recycle_To_Rack, 1);
    if (sample->nextProgram().isEmpty()) {
        sample->setState(SampleStatus::Test_Finished, 1);
    }
    sample->setState(SampleStatus::Review_Done, 1);
}

void MStationSmear::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api.startsWith("S2") == false) {
        return;
    }

    if (api == "S2_Scan_Open") {
        mScanBarcode.clear();
        if (resValue.isObject()) {
            QJsonObject obj = resValue.toObject();
            mScanBarcode = obj.value("string").toString().simplified();
        }
    }
}
