#include "device_item.h"
#include "ui_device_item.h"

#include "command/debug_command_dialog.h"
#include "device/rt_device_base.h"
#include "device/rt_device_manager.h"
#include "module_manager.h"
#include "f_common.h"

DeviceItem::DeviceItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceItem)
{
    ui->setupUi(this);

    m_debugDialog = new DebugCommandDialog(this);

    init();
}

DeviceItem::~DeviceItem()
{
    delete ui;
}

void DeviceItem::init()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->label_alarm->hide();
}

void DeviceItem::reset()
{
    init();
}

void DeviceItem::setDeviceInfo(const QString &name, const QString &address)
{
    ui->label_device_name->setText(name);
    ui->label_address->setText(address);
}

void DeviceItem::setDevice(RtDeviceBase *device)
{
    if (device) {
        m_device = device;
        connect(device, &RtDeviceBase::sendPageInfo_signal,
                this, &DeviceItem::handleInfo_slot);
        connect(device, &RtDeviceBase::sonserInfo_signal,
                this, &DeviceItem::handleSensorInfo_slot);

        connect(m_debugDialog, &DebugCommandDialog::sendCommand_signal,
                device, &RtDeviceBase::exeFunction_slot);

        m_debugDialog->setTitle(device->deviceID());
        m_debugDialog->addFunctionList(device->getCommandList(), Arg_None);
        m_debugDialog->addFunctionList(device->getCommandArg1List(), Arg_Integer_1);
        m_debugDialog->addFunctionList(device->getCommandArg2List(), Arg_Integer_2);
        m_debugDialog->addFunctionList(device->getCommandArg3List(), Arg_Integer_3);
        m_debugDialog->addFunctionList(device->getCommandArg4List(), Arg_Integer_4);
        m_debugDialog->addFunctionList(device->getCommandArgSList(), Arg_String);
        m_debugDialog->addFunctionList(device->getCommandArgISList(), Arg_Int_String);
    }
}

void DeviceItem::showDebugDialog()
{
    m_debugDialog->show();
}

void DeviceItem::handleInfo_slot(const QString &dev_id, int code, const QString &message)
{
    Q_UNUSED(dev_id)

    if (code > 0) {
        ui->label_alarm->show();
        ui->label_alarm->setToolTip(QString("code:%1, %2").arg(code).arg(message));
    }
}

void DeviceItem::handleSensorInfo_slot(const QString &dev_id, const QJsonObject &obj)
{
    Q_UNUSED(dev_id)
    Q_UNUSED(obj)

    ui->widget_heartbeat->showOnce();
}

void DeviceItem::on_pushButton_start_clicked()
{
    DModuleManager::GetInstance()->startOne(m_device->deviceID());
}

void DeviceItem::on_pushButton_pause_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_btn_pause);
}

void DeviceItem::on_pushButton_resume_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_btn_resume);
}

void DeviceItem::on_pushButton_stop_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_btn_start);
    DModuleManager::GetInstance()->stop(m_device->deviceID());
}

void DeviceItem::on_pushButton_reset_clicked()
{
    m_device->reset();
    DModuleManager::GetInstance()->reset(m_device->deviceID());
}

void DeviceItem::on_pushButton_tool_clicked()
{
    m_debugDialog->show();
}
