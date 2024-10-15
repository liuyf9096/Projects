#include "m_station1.h"
#include "settings/f_settings.h"

MStation1::MStation1(QObject *parent)
    : MStation{"station1", "S1", 10, 12, parent}
{
    auto settings = FSettings::GetInstance();
    mClientDevid = settings->station1_deviceid();
    m_isUnited = settings->isStation1_united();

    ProgramList << "5-classify" << "crp";
    qDebug() << QString("Station1 [%1] program:").arg(mClientDevid) << ProgramList;

    auto track1 = RtDeviceManager::GetInstance()->track1();
    connect(track1, &DTrack1::onFunctionFinished_signal,
            this, &MStation1::onFunctionFinished_slot);
}

void MStation1::setUnited(bool isUnited)
{
    if (m_isUnited != isUnited) {
        FSettings::GetInstance()->setStation1_United(isUnited);
        m_isUnited = isUnited;
        qDebug() << QString("Station1 [%1] setUnited:").arg(mClientDevid) << isUnited;
        updateStationStatus();
    }
}

void MStation1::setClientId(const QString &clientid)
{
    FSettings::GetInstance()->setStation1_Device_Address(clientid);
}

void MStation1::cmd_Scan_Open()
{
    dev->track1()->cmd_S1_Scan_Open();
}

void MStation1::cmd_Scan_Close()
{
    dev->track1()->cmd_S1_Scan_Close();
}

void MStation1::cmd_Emergency_Open()
{
    dev->track1()->cmd_S1_Emergency_Open();
}

void MStation1::cmd_Emergency_Close()
{
    dev->track1()->cmd_S1_Emergency_Close();
}

void MStation1::cmd_CheckSensorValue()
{
    dev->track1()->cmd_CheckSensorValue();
}

bool MStation1::isScanOpen_Done()
{
    return dev->track1()->isFuncDone("S1_Scan_Open");
}

bool MStation1::isScanClose_Done()
{
    return dev->track1()->isFuncDone("S1_Scan_Close");
}

bool MStation1::isEmergencyOpen_Done()
{
    return dev->track1()->isFuncDone("S1_Emergency_Open");
}

bool MStation1::isEmergencyClose_Done()
{
    return dev->track1()->isFuncDone("S1_Emergency_Close");
}

bool MStation1::isCheckSensorValue_Done()
{
    return dev->track1()->isFuncDone("CheckSensorValue");
}

bool MStation1::isSampleExist()
{
    return dev->track1()->checkSensorValue("S1_scan_sample_exist");
}

void MStation1::receiveFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Send_To_Test, 1);
    sample->setState(SampleStatus::SamplingFinished, 0);
    sample->setState(SampleStatus::Recycle_To_Rack, 0);
}

void MStation1::recycleFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Test_Finished, 1);
    sample->setState(SampleStatus::Recycle_To_Rack, 1);
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
    } else if (api == "S1_Emergency_Open") {
        JPacket p(PacketType::Result, closetOpen_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    } else if (api == "S1_Emergency_Close") {
        JPacket p(PacketType::Result, closetClose_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    }
}
