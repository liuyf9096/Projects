#ifndef SENSOR_ITEM_H
#define SENSOR_ITEM_H

#include <QWidget>

namespace Ui {
class SensorItem;
}

class SensorItem : public QWidget
{
    Q_OBJECT

public:
    explicit SensorItem(QWidget *parent = nullptr);
    ~SensorItem();

    void setInfo(const QString &name, const QString &port);
    void setSensorOn(bool on);

private:
    Ui::SensorItem *ui;
};

#endif // SENSOR_ITEM_H
