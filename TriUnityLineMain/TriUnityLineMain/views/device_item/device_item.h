#ifndef DEVICE_ITEM_H
#define DEVICE_ITEM_H

#include <QWidget>

namespace Ui {
class DeviceItem;
}

class RtDeviceBase;
class DebugCommandDialog;
class DeviceItem : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceItem(QWidget *parent = nullptr);
    ~DeviceItem();

    void reset();
    void setFunctionEn(const bool en);
    void setDeviceInfo(const QString &name, const QString &address);

    void setDevice(RtDeviceBase *device);

    void showDebugDialog();

public slots:
    void handleInfo_slot(const QString &dev_id, int code, const QString &message);
    void handleSensorInfo_slot(const QString &dev_id, const QJsonObject &obj);

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_pause_clicked();
    void on_pushButton_resume_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_tool_clicked();

private:
    Ui::DeviceItem *ui;

    RtDeviceBase *m_device;
    DebugCommandDialog *m_debugDialog;

     void init();
};

#endif // DEVICE_ITEM_H
