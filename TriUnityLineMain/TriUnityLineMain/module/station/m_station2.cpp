#include "m_station2.h"
#include "settings/f_settings.h"

MStation2::MStation2(QObject *parent)
    : MStation{"station2", "S2", 29, 31, parent}
{
    auto settings = FSettings::GetInstance();
    mClientDevid = settings->station2_deviceid();
    m_isUnited = settings->isStation2_united();

    ProgramList << "5-classify" << "crp";
    qDebug() << QString("Station2 [%1] program:").arg(mClientDevid) << ProgramList;

    auto track2 = RtDeviceManager::GetInstance()->track2();
    connect(track2, &DTrack2::onFunctionFinished_signal,
            this, &MStation2::onFunctionFinished_slot);
}

void MStation2::setUnited(bool isUnited)
{
    if (m_isUnited != isUnited) {
        FSettings::GetInstance()->setStation2_United(isUnited);
        m_isUnited = isUnited;
        qDebug() << QString("Station2 [%1] setUnited:").arg(mClientDevid) << isUnited;
        updateStationStatus();
    }
}

void MStation2::setClientId(const QString &clientid)
{
    FSettings::GetInstance()->setStation2_Device_Address(clientid);
}

void MStation2::cmd_Scan_Open()
{
    dev->track2()->cmd_S2_Scan_Open();
}

void MStation2::cmd_Scan_Close()
{
    dev->track2()->cmd_S2_Scan_Close();
}

void MStation2::cmd_Emergency_Open()
{
    dev->track2()->cmd_S2_Emergency_Open();
}

void MStation2::cmd_Emergency_Close()
{
    dev->track2()->cmd_S2_Emergency_Close();
}

void MStation2::cmd_CheckSensorValue()
{
    dev->track2()->cmd_CheckSensorValue();
}

bool MStation2::isScanOpen_Done()
{
    return dev->track2()->isFuncDone("S2_Scan_Open");
}

bool MStation2::isScanClose_Done()
{
    return dev->track2()->isFuncDone("S2_Scan_Close");
}

bool MStation2::isEmergencyOpen_Done()
{
    return dev->track2()->isFuncDone("S2_Emergency_Open");
}

bool MStation2::isEmergencyClose_Done()
{
    return dev->track2()->isFuncDone("S2_Emergency_Close");
}

bool MStation2::isCheckSensorValue_Done()
{
    return dev->track2()->isFuncDone("CheckSensorValue");
}

bool MStation2::isSampleExist()
{
    return dev->track2()->checkSensorValue("S2_scan_sample_exist");
}

void MStation2::receiveFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Send_To_Test, 1);
    sample->setState(SampleStatus::SamplingFinished, 0);
    sample->setState(SampleStatus::Recycle_To_Rack, 0);
}

void MStation2::recycleFinished(QSharedPointer<RtSample> sample)
{
    sample->setState(SampleStatus::Test_Finished);
    sample->setState(SampleStatus::Recycle_To_Rack);
}

void MStation2::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
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
    } else if (api == "S2_Emergency_Open") {
        JPacket p(PacketType::Result, closetOpen_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    } else if (api == "S2_Emergency_Close") {
        JPacket p(PacketType::Result, closetClose_id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendClientMessage(mClientid, p);
    }
}
