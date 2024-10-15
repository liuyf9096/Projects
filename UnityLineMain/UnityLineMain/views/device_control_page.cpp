#include "device_control_page.h"
#include "ui_device_control_page.h"

#include "device/rt_device_manager.h"
#include "module/module_manager.h"
#include "sample/rt_sample_manager.h"
#include "device_item/device_item.h"
#include "servers/websocket/f_websocket_server.h"
#include "exception/exception_center.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>

DeviceControlPage::DeviceControlPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceControlMainPage)
{
    ui->setupUi(this);

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onDMUMessageResultPacket_signal,
            this, &DeviceControlPage::handleSampleOrder_slot);
}

DeviceControlPage::~DeviceControlPage()
{
    delete ui;
}

void DeviceControlPage::configDeviceList()
{
    auto list = RtDeviceManager::GetInstance()->deviceList();
    for (int i = 0; i < list.count(); ++i) {
        auto device = list.at(i);
        if (device->isEnable()) {
            QString dev_id = device->deviceID();
            QString username = device->username();
            int address = device->address();

            addNewForm(dev_id, username, address);
        }
    }
}

void DeviceControlPage::addNewForm(const QString &dev_id, const QString &name, int address)
{
    DeviceItem *form = new DeviceItem(this);

    QString addr = QString("0x%1").arg(address, 2, 16, QChar('0'));
    form->setDeviceInfo(name, addr);

    RtDeviceBase *device = RtDeviceManager::GetInstance()->getDevice(dev_id);
    form->setDevice(device);

    auto item = new QListWidgetItem();
    item->setSizeHint(QSize(200, 80));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, form);
    m_formList.append(form);
}

void DeviceControlPage::on_pushButton_start_clicked()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

    DModuleManager::GetInstance()->start();
}

void DeviceControlPage::on_pushButton_stop_clicked()
{
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_start->setEnabled(true);

    DModuleManager::GetInstance()->stop();
}

void DeviceControlPage::on_pushButton_reset_clicked()
{
    if (FMessageCenter::GetInstance()->isCanbusConnected()) {
        RtDeviceManager::GetInstance()->sendAllDeviceReset();
        DModuleManager::GetInstance()->reset();

        foreach (auto form, m_formList) {
            form->reset();
        }
    } else {
        qDebug() << "can NOT reset device, please connect the device first.";
    }
}

void DeviceControlPage::on_pushButton_1_clicked()
{
#if 0
    JPacket p(PacketType::Request);
    p.device = "LabXpert";
    p.module = "Order";
    p.api = "AssignOrderUID";

    QJsonObject obj;
    obj.insert("barcode", "");
    obj.insert("rack_num", "R1");
    obj.insert("tube_num", 5);
    p.paramsValue = obj;
#endif
    JPacket p(PacketType::Notification);
    p.device = "UnitedUI";
    p.module = "Exception";
    p.api = "Exception";

    QJsonObject obj;
    obj.insert("error_id", "Reset_error");
    obj.insert("message", "Reset_error_80");
    obj.insert("error_level", 2);
    obj.insert("solution", "Reset again");
    obj.insert("dev_id", "United");
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

#if 0
void DeviceControlPage::on_pushButton_2_clicked()
{
    JPacket p(PacketType::Request, 2);
    p.device = "BF800";
    p.module = "Sample";
    p.api = "TakeUpNewSample";

    QJsonObject obj;
    obj.insert("sample_uid", "690685542");
    obj.insert("rack_id", "R3");
    obj.insert("rack_pos", 6);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendClientMessage("", p);
}

void DeviceControlPage::on_pushButton_3_clicked()
{
    JPacket p(PacketType::Request, 0);
    p.device = "BF800";
    p.module = "Sample";
    p.api = "RecycleSampleToRack";

    QJsonObject obj;
    obj.insert("sample_uid", "690685542");
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendClientMessage("", p);
}

void DeviceControlPage::on_pushButton_4_clicked()
{
    JPacket p(PacketType::Request, 4);
    p.device = "BF800";
    p.module = "Sample";
    p.api = "RotateTube";

    FMessageCenter::GetInstance()->sendClientMessage("", p);

#if 0
    JPacket p(PacketType::Request);
    p.device = "LabXpert";
    p.module = "Order";
    p.api = "AssignOrderUID";

    QJsonObject obj;
    obj.insert("barcode", "");
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendDMUMessage(p);
#endif
}

#endif

void DeviceControlPage::on_pushButton_2_clicked()
{
#if 0
    JPacket p(PacketType::Request);
    p.device = "LabXpert";
    p.module = "Order";
    p.api = "AssignTestUID";

    QJsonObject obj;
    obj.insert("order_uid", m_order_uid);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendDMUMessage(p);
#endif
    JPacket p(PacketType::Notification);
    p.device = "UnitedUI";
    p.module = "Exception";
    p.api = "ClearException";

    QJsonObject obj;
    obj.insert("error_id", "Reset_error");
    obj.insert("dev_id", "United");
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void DeviceControlPage::handleSampleOrder_slot(const JPacket &result, const JPacket &request)
{
    if (request.api == "AssignOrderUID") {
        QJsonObject obj = result.resValue.toObject();
        qDebug() << ">>" << obj;
        m_order_uid = obj.value("order_uid").toString();
        on_pushButton_2_clicked();
    } else if (request.api == "AssignTestUID") {
        QJsonObject obj = result.resValue.toObject();
        qDebug() << ">>" << obj;
    }
}

