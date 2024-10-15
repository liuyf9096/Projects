#include "sensors_form.h"
#include "ui_sensors_form.h"

#include "sensor_item.h"
#include "device/rt_device_base.h"

#include <QJsonArray>
#include <QLayout>

SensorsForm::SensorsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorsForm)
{
    ui->setupUi(this);
}

SensorsForm::~SensorsForm()
{
    delete ui;
}

void SensorsForm::setDeviceInfo(const QString &name, const bool sensorOn)
{
    ui->groupBox->setTitle(name);
    ui->checkBox_sensorEn->setChecked(sensorOn);
}

void SensorsForm::setSensorPort(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();

        int port = obj.value("port").toInt();
        QString name = obj.value("name").toString();

        SensorItem *item = new SensorItem(this);
        item->setInfo(name, QString::number(port));
        m_portMap.insert(QString::number(port), item);

        int count = ui->listLayout->count();
        ui->listLayout->insertWidget(count - 1, item);
    }
}

void SensorsForm::setDevice(RtDeviceBase *device)
{
    if (device != nullptr) {
        m_device = device;
        connect(device, &RtDeviceBase::sonserInfo_signal,
                this, &SensorsForm::handleSensorData_slot);
    }
}

void SensorsForm::handleSensorData_slot(const QString &dev_id, const QJsonObject &obj)
{
    Q_UNUSED(dev_id)

    if (ui->checkBox_sensorEn->isChecked()) {
        foreach (auto port, obj.keys()) {
            if (m_portMap.contains(port)) {
                auto item = m_portMap.value(port);
                bool on = obj.value(port).toBool();
                item->setSensorOn(on);
            }
        }
    }
}

void SensorsForm::on_checkBox_sensorEn_clicked()
{
    if (ui->checkBox_sensorEn->isChecked()) {
        if (m_device) {
            m_device->setCheckSensorEnable("form", true);
        }
    } else {
        if (m_device) {
            m_device->setCheckSensorEnable("form", false);
        }
        resetAllItems();
    }
}

void SensorsForm::resetAllItems()
{
    foreach (auto item, m_portMap) {
        item->setSensorOn(false);
    }
}
