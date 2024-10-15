#ifndef DEVICE_CONTROL_PAGE_H
#define DEVICE_CONTROL_PAGE_H

#include "messagecenter/f_message_center.h"
#include <QWidget>

namespace Ui {
class DeviceControlMainPage;
}

class QTimer;
class DeviceItem;
class DeviceControlPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceControlPage(QWidget *parent = nullptr);
    ~DeviceControlPage();

    void configDeviceList();

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_reset_clicked();

    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::DeviceControlMainPage *ui;

    void addNewForm(const QString &dev_id, const QString &name, int address);
    QList<DeviceItem*> m_formList;

    QString m_order_uid;
    QString m_test_uid;

private slots:
    void handleSampleOrder_slot(const JPacket &result, const JPacket &request);
};

#endif // DEVICE_CONTROL_PAGE_H
