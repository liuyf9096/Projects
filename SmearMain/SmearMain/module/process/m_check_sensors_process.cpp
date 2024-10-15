#include "m_check_sensors_process.h"
#include "f_common.h"

MCheckSensorsProcess::MCheckSensorsProcess(QObject *parent)
    : MProcessBase{"checksensors", "checksensors", parent}
{
    _init();
    _getFloaterId();

    connect(RtDeviceManager::GetInstance()->sample(), &DSample::onFunctionFinished_signal,
            this, &MCheckSensorsProcess::handleSampleFunctionFinish_slot);
    connect(RtDeviceManager::GetInstance()->sample(), &DSample::floaterSensorInfo_signal,
            this, &MCheckSensorsProcess::handleSampleFloaterSensorNote_slot);
    connect(RtDeviceManager::GetInstance()->stain(), &DStain::onFunctionFinished_signal,
            this, &MCheckSensorsProcess::handleStainFunctionFinish_slot);
    connect(RtDeviceManager::GetInstance()->stain(), &DSample::floaterSensorInfo_signal,
            this, &MCheckSensorsProcess::handleStainFloaterSensorNote_slot);
}

void MCheckSensorsProcess::_init()
{
    s_floater = FloaterState::WaitF_Check_FloatSensor;
}

void MCheckSensorsProcess::_getFloaterId()
{
    bool sample_en = FCommon::GetInstance()->getConfigValue("check", "sample", "enable").toBool();
    if (sample_en) {
        QJsonArray arr = FCommon::GetInstance()->getConfigValue("check", "sample", "floater").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject obj = arr.at(i).toObject();
            int port = obj.value("port").toInt();
            QString id = obj.value("id").toString();
            int alarm_value = obj.value("alarm_value").toInt();
            m_SampleFloaterIdMap.insert(port, id);
            m_FloaterAlarmValueMap.insert(id, alarm_value);
        }
    }
    qDebug() << "module:sample Floater Sensor:" << m_SampleFloaterIdMap;

    bool stain_en = FCommon::GetInstance()->getConfigValue("check", "stain", "enable").toBool();
    if (stain_en) {
        QJsonArray arr = FCommon::GetInstance()->getConfigValue("check", "stain", "floater").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject obj = arr.at(i).toObject();
            int port = obj.value("port").toInt();
            QString id = obj.value("id").toString();
            int alarm_value = obj.value("alarm_value").toInt();
            m_StainFloaterIdMap.insert(port, id);
            m_FloaterAlarmValueMap.insert(id, alarm_value);
        }
    }
    qDebug() << "module:stain Floater Sensor:" << m_StainFloaterIdMap;
    qDebug() << "Floater Sensor alarm value:" << m_FloaterAlarmValueMap;
}

bool MCheckSensorsProcess::startProcess()
{
    if (m_SampleFloaterIdMap.isEmpty() == false) {
        dev->sample()->setCheckFloaterSensorEnable(true, m_SampleFloaterIdMap.count());
    }
    if (m_StainFloaterIdMap.isEmpty() == false) {
        dev->stain()->setCheckFloaterSensorEnable(true, m_StainFloaterIdMap.count());
    }
    if (mTimer->isActive() == false) {
        mTimer->start(2000);
    }
    return true;
}

void MCheckSensorsProcess::checkOnce()
{
    startProcess();
    state_init();
    m_sendAlarmList.clear();
}

void MCheckSensorsProcess::startCheck()
{
    startProcess();
    state_init();
    m_sendAlarmList.clear();
}

void MCheckSensorsProcess::state_init()
{
    _init();
}

void MCheckSensorsProcess::onTimer_slot()
{
    checkFloaterProc();
}

void MCheckSensorsProcess::checkFloaterProc()
{
    switch (s_floater) {
    case FloaterState::WaitF_Check_FloatSensor:
        if (m_sendSampleAlarmList == m_sampleErrorList
                && m_sendStainAlarmList == m_stainErrorList) {
            s_floater = FloaterState::WaitF_Check_FloatSensor;
        } else {
            s_floater = FloaterState::Send_Alarm;
        }
        break;

    case FloaterState::Send_Alarm:
        sendAlarm();
        clearAlarm();
        s_floater = FloaterState::WaitF_Check_FloatSensor;
        break;

    case FloaterState::Finish:
        break;
    default:
        break;
    }
}

void MCheckSensorsProcess::sendAlarm()
{
    if (m_sendSampleAlarmList != m_sampleErrorList) {
        JPacket p(PacketType::Notification);
        p.module = "Exception";
        p.api = "SetException";

        QJsonObject obj;
        obj.insert("source", "MidCtrl");
        obj.insert("message", "floater alarm.");

        QJsonArray arr;
        for (int i = 0; i < m_sampleErrorList.count(); ++i) {
            arr.append(m_sampleErrorList.at(i));
        }
        obj.insert("error_id", arr);
        p.paramsValue = obj;
        FMessageCenter::GetInstance()->sendUIMessage(p);

        m_sendSampleAlarmList = m_sampleErrorList;
    }
    if (m_sendStainAlarmList != m_stainErrorList) {
        JPacket p(PacketType::Notification);
        p.module = "Exception";
        p.api = "SetException";

        QJsonObject obj;
        obj.insert("source", "MidCtrl");
        obj.insert("message", "floater alarm.");

        QJsonArray arr;
        for (int i = 0; i < m_stainErrorList.count(); ++i) {
            arr.append(m_stainErrorList.at(i));
        }
        obj.insert("error_id", arr);
        p.paramsValue = obj;
        FMessageCenter::GetInstance()->sendUIMessage(p);

        m_sendStainAlarmList = m_stainErrorList;
    }
}

void MCheckSensorsProcess::clearAlarm()
{
    QStringList clearList;
    foreach (auto floater, m_SampleFloaterIdMap) {
        if (m_sendSampleAlarmList.contains(floater) == false) {
            clearList.append(floater);
        }
    }
    foreach (auto floater, m_StainFloaterIdMap) {
        if (m_sendStainAlarmList.contains(floater) == false) {
            clearList.append(floater);
        }
    }
    if (clearList.isEmpty() == false) {
        JPacket p(PacketType::Notification);
        p.module = "Exception";
        p.api = "ClearException";

        QJsonObject obj;
        obj.insert("source", "MidCtrl");
        obj.insert("message", "floater alarm.");

        QJsonArray arr;
        for (int i = 0; i < clearList.count(); ++i) {
            arr.append(clearList.at(i));
        }
        obj.insert("error_id", arr);
        p.paramsValue = obj;
        FMessageCenter::GetInstance()->sendUIMessage(p);
    }
}

void MCheckSensorsProcess::handleSampleFunctionFinish_slot(const QString &api, const QJsonValue &resValue)
{
    if (api == "QueryAllFloater") {
        QJsonObject resObj = resValue.toObject();
        int count = resObj.value("count").toInt();
#if 0
        if (resObj.contains("floater")) {
            QJsonObject floaterObj = resObj.value("floater").toObject();
            for (int i = 0; i < count; ++i) {
                QString floater = m_SampleFloaterIdMap.value(i);
                if (floaterObj.contains(floater)) {
                    int value = floaterObj.value(floater).toInt();
                    int e_standard = m_FloaterAlarmValueMap.value(floater);
                    if (value == e_standard) {
                        m_errorList.append(floater);
                    }
                }
            }
        }
#endif
        if (resObj.contains("values")) {
            QJsonArray arr = resObj.value("values").toArray();
            if (arr.count() == count) {
                for (int i = 0; i < arr.count(); ++i) {
                    int value = arr.at(i).toInt();
                    if (m_SampleFloaterIdMap.contains(i)) {
                        QString floater_id = m_SampleFloaterIdMap.value(i);
                        if (m_FloaterAlarmValueMap.contains(floater_id)) {
                            int alarm_value = m_FloaterAlarmValueMap.value(floater_id);
                            if (value == alarm_value) {
                                m_errorList.append(floater_id);
                            }
                        }
                    }
                }
            }
        }
    }
}

void MCheckSensorsProcess::handleStainFunctionFinish_slot(const QString &api, const QJsonValue &resValue)
{
    if (api == "QueryAllFloater") {
        QJsonObject resObj = resValue.toObject();
        int count = resObj.value("count").toInt();
        if (resObj.contains("values")) {
            QJsonArray arr = resObj.value("values").toArray();
            if (arr.count() == count) {
                for (int i = 0; i < arr.count(); ++i) {
                    int value = arr.at(i).toInt();
                    if (m_StainFloaterIdMap.contains(i)) {
                        QString floater_id = m_StainFloaterIdMap.value(i);
                        if (m_FloaterAlarmValueMap.contains(floater_id)) {
                            int alarm_value = m_FloaterAlarmValueMap.value(floater_id);
                            if (value == alarm_value) {
                                m_errorList.append(floater_id);
                            }
                        }
                    }
                }
            }
        }
    }
}

void MCheckSensorsProcess::handleSampleFloaterSensorNote_slot(const QString &dev_id, const QJsonObject &obj)
{
    Q_UNUSED(dev_id)

    m_sampleErrorList.clear();

    int count = obj.value("count").toInt();
    if (obj.contains("floater")) {
        QJsonObject floaterObj = obj.value("floater").toObject();
        for (int i = 0; i < count; ++i) {
            QString floater_id = m_SampleFloaterIdMap.value(i);
            if (floaterObj.contains(floater_id)) {
                int value = floaterObj.value(floater_id).toInt();
                int e_standard = m_FloaterAlarmValueMap.value(floater_id);
                if (value == e_standard) {
                    m_sampleErrorList.append(floater_id);
                }
            }
        }
    } else if (obj.contains("values")) {
        QJsonArray arr = obj.value("values").toArray();
        if (arr.count() == count) {
            for (int i = 0; i < arr.count(); ++i) {
                int value = arr.at(i).toInt();
                if (m_SampleFloaterIdMap.contains(i)) {
                    QString floater_id = m_SampleFloaterIdMap.value(i);
                    if (m_FloaterAlarmValueMap.contains(floater_id)) {
                        int alarm_value = m_FloaterAlarmValueMap.value(floater_id);
                        if (value == alarm_value) {
                            m_sampleErrorList.append(floater_id);
                        }
                    }
                }
            }
        }
    }
}

void MCheckSensorsProcess::handleStainFloaterSensorNote_slot(const QString &dev_id, const QJsonObject &obj)
{
    Q_UNUSED(dev_id)

    m_stainErrorList.clear();

    int count = obj.value("count").toInt();

    if (obj.contains("floater")) {
        QJsonObject floaterObj = obj.value("floater").toObject();
        for (int i = 0; i < count; ++i) {
            QString floater_id = m_SampleFloaterIdMap.value(i);
            if (floaterObj.contains(floater_id)) {
                int value = floaterObj.value(floater_id).toInt();
                int e_standard = m_FloaterAlarmValueMap.value(floater_id);
                if (value == e_standard) {
                    m_sampleErrorList.append(floater_id);
                }
            }
        }
    } else if (obj.contains("values")) {
        QJsonArray arr = obj.value("values").toArray();
        if (arr.count() == count) {
            for (int i = 0; i < arr.count(); ++i) {
                int value = arr.at(i).toInt();
                if (m_StainFloaterIdMap.contains(i)) {
                    QString floater_id = m_StainFloaterIdMap.value(i);
                    if (m_FloaterAlarmValueMap.contains(floater_id)) {
                        int alarm_value = m_FloaterAlarmValueMap.value(floater_id);
                        if (value == alarm_value) {
                            m_stainErrorList.append(floater_id);
                        }
                    }
                }
            }
        }
    }
}
