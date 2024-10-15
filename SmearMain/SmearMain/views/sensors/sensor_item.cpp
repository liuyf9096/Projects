#include "sensor_item.h"
#include "ui_sensor_item.h"

SensorItem::SensorItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SensorItem)
{
    ui->setupUi(this);
}

SensorItem::~SensorItem()
{
    delete ui;
}

void SensorItem::setInfo(const QString &name, const QString &port)
{
    ui->label_name->setText(name);
    ui->label_port->setText(port);
}

void SensorItem::setSensorOn(bool on)
{
    ui->checkBox_sensorOn->setChecked(on);
}
