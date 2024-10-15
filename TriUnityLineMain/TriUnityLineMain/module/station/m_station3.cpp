#include "m_station3.h"
#include "settings/f_settings.h"

MStation3::MStation3(QObject *parent)
    : MStation{"station3", "S3", 50, 52, parent}
{
    auto settings = FSettings::GetInstance();
    mClientDevid = settings->station3_deviceid();
    m_isUnited = settings->isStation3_united();

    ProgramList << "smear";
    qDebug() << QString("Station3 [%1] program:").arg(mClientDevid) << ProgramList;

    auto track2 = RtDeviceManager::GetInstance()->track2();
    connect(track2, &DTrack2::onFunctionFinished_signal,
            this, &MStation3::onFunctionFinished_slot);
}

void MStation3::setUnited(bool isUnited)
{
    if (m_isUnited != isUnited) {
        FSettings::GetInstance()->setStation3_United(isUnited);
        m_isUnited = isUnited;
        qDebug() << QString("Station3 [%1] setUnited:").arg(mClientDevid) << isUnited;
        updateStationStatus();
    }
}

void MStation3::setClientId(const QString &clientid)
{
    FSettings::GetInstance()->setStation3_Device_Address(clientid);
}

void MStation3::cmd_Scan_Open()
{
    dev->track2()->cmd_S3_Scan_Open();
}

void MStation3::cmd_Scan_Close()
{
    dev->track2()->cmd_S3_Scan_Close();
}

void MStation3::cmd_Emergency_Open()
{
    dev->track2()->cmd_S3_Emergency_Open();
}

void MStation3::cmd_Emergency_Close()
{
    dev->track2()->cmd_S3_Emergency_Close();
}

void MStation3::cmd_CheckSensorValue()
{
    dev->track2()->cmd_CheckSensorValue();
}

bool MStation3::isScanOpen_Done()
{
    return dev->track1()->isFuncDone("S3_Scan_Open");
}

bool MStation3::isScanClose_Done()
{
    return dev->track1()->isFuncDone("S3_Scan_Close");
}

bool MStation3::isEmergencyOpen_Done()
{
    return dev->track2()->isFuncDone("S3_Emergency_Open");
}

bool MStation3::isEmergencyClose_Done()
{
    return dev->track2()->isFuncDone("S3_Emergency_Close");
}

bool MStation3::isCheckSensorValue_Done()
{
    return dev->track2()->isFuncDone("CheckSensorValue");
}

bool MStation3::isSampleExist()
{
    return dev->track2()->checkSensorValue("S3_scan_sample_exist");
}

void MStation3::receiveFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Send_To_Smear, 1);
    sample->setState(SampleStatus::SamplingFinished, 0);
    sample->setState(SampleStatus::Recycle_To_Rack, 0);
}

void MStation3::recycleFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Smear_Done);
    sample->setState(SampleStatus::Recycle_To_Rack);
}

void MStation3::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api.startsWith("S3") == false) {
        return;
    }

    if (api == "S3_Scan_Open") {
        mScanBarcode.clear();
        if (resValue.isObject()) {
            QJsonObject obj = resValue.toObject();
            mScanBarcode = obj.value("string").toString().simplified();
        }
    } else if (api == "S3_Emergency_Open") {
        JPacket p(PacketType::Result, closetOpen_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    } else if (api == "S3_Emergency_Close") {
        JPacket p(PacketType::Result, closetClose_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    }
}
