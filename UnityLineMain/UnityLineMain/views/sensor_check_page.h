#ifndef SENSOR_CHECK_PAGE_H
#define SENSOR_CHECK_PAGE_H

#include <QWidget>

namespace Ui {
class SensorCheckForm;
}

class SensorCheckPage : public QWidget
{
    Q_OBJECT

public:
    explicit SensorCheckPage(QWidget *parent = nullptr);
    ~SensorCheckPage();

    void configforms();

private:
    Ui::SensorCheckForm *ui;
};

#endif // SENSOR_CHECK_PAGE_H
