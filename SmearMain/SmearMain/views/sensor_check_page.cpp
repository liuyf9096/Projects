#include "sensor_check_page.h"
#include "ui_sensor_check_page.h"

#include "sensors/sensors_form.h"
#include "device/rt_device_manager.h"

#include <QJsonObject>
#include <QDebug>

SensorCheckPage::SensorCheckPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorCheckForm)
{
    ui->setupUi(this);

    configforms();
}

SensorCheckPage::~SensorCheckPage()
{
    delete ui;
}

void SensorCheckPage::configforms()
{
    auto manager = RtDeviceManager::GetInstance();
    auto list = manager->deviceList();

    for (int i = 0; i < list.count(); ++i) {
        auto device = list.at(i);
        bool enable = device->getCheckSensorEnable();

        SensorsForm *form = new SensorsForm(this);
        form->setDeviceInfo(device->username(), enable);
        form->setDevice(device);
        form->setSensorPort(device->getSensorArr());

        ui->scrollLayout->addWidget(form);
    }
}
