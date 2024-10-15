#ifndef DEVICE_SETTING_PAGE_H
#define DEVICE_SETTING_PAGE_H

#include <QWidget>

namespace Ui {
class DeviceControlMainPage;
}

class QTimer;
class DeviceItem;
class DeviceSettingPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSettingPage(QWidget *parent = nullptr);
    ~DeviceSettingPage();

    void configDeviceList();

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_checknew_clicked();

    void on_pushButton_test_clicked();

private:
    Ui::DeviceControlMainPage *ui;

    void addNewForm(const QString &dev_id, const QString &name, int address);
    QList<DeviceItem*> m_formList;
};

#endif // DEVICE_SETTING_PAGE_H
