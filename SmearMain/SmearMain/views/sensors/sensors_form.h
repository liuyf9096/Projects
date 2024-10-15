#ifndef SENSORS_FORM_H
#define SENSORS_FORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class SensorsForm;
}

class RtDeviceBase;
class SensorItem;
class SensorsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SensorsForm(QWidget *parent = nullptr);
    ~SensorsForm();

    void setDeviceInfo(const QString &name, const bool sensorOn);
    void setSensorPort(const QJsonArray &arr);
    void setDevice(RtDeviceBase *device);

private:
    Ui::SensorsForm *ui;

    RtDeviceBase *m_device;
    QMap<QString, SensorItem*> m_portMap;

    void resetAllItems();

private slots:
    void on_checkBox_sensorEn_clicked();
    void handleSensorData_slot(const QString &dev_id, const QJsonObject &obj);
};

#endif // SENSORS_FORM_H
