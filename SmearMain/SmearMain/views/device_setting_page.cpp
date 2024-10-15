#include "device_setting_page.h"
#include "ui_device_setting_page.h"
#include "device/rt_device_manager.h"
#include "module_manager.h"
#include "messagecenter/f_message_center.h"
#include "device_item/device_item.h"
#include "track/track_manager.h"
#include "sample/rt_sample_manager.h"
#include "stain/stain_manager.h"
#include "smear/smear_manager.h"
#include "process/process_manager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>

DeviceSettingPage::DeviceSettingPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceControlMainPage)
{
    ui->setupUi(this);
}

DeviceSettingPage::~DeviceSettingPage()
{
    delete ui;
}

void DeviceSettingPage::configDeviceList()
{
    auto list = RtDeviceManager::GetInstance()->deviceList();
    for (int i = 0; i < list.count(); ++i) {
        auto device = list.at(i);

        QString dev_id = device->deviceID();
        QString username = device->username();
        int address = device->address();

        addNewForm(dev_id, username, address);
    }
}

void DeviceSettingPage::addNewForm(const QString &dev_id, const QString &name, int address)
{
    DeviceItem *form = new DeviceItem(this);

    QString addr = QString("0x%1").arg(address, 2, 16, QChar('0'));
    form->setDeviceInfo(name, addr);

    RtDeviceBase *device = RtDeviceManager::GetInstance()->getDevice(dev_id);
    form->setDevice(device);

    auto item = new QListWidgetItem();
    item->setSizeHint(QSize(200, 100));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, form);
    m_formList.append(form);
}

void DeviceSettingPage::on_pushButton_start_clicked()
{
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

    DModuleManager::GetInstance()->start();
    MProcessManager::GetInstance()->mCheckSensorsProc->startProcess();
}

void DeviceSettingPage::on_pushButton_stop_clicked()
{
    ui->pushButton_stop->setEnabled(false);
    ui->pushButton_start->setEnabled(true);

    DModuleManager::GetInstance()->stop();
}

void DeviceSettingPage::on_pushButton_reset_clicked()
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

void DeviceSettingPage::on_pushButton_checknew_clicked()
{
#if 1
    DModuleManager::GetInstance()->startAutoTest();
    MRecycleBoxMgr::GetInstance()->startRun(true);
#endif

#if 0
    QJsonArray arr;
    for (int i = 0; i < 3; ++i) {
        QJsonObject obj;
        obj.insert("sample_id", QString("S-%1").arg(i));
        QJsonObject printObj;
        printObj.insert("line1", QString("S-%1").arg(i));
        printObj.insert("line2", "date");
        printObj.insert("line3", "time");
        obj.insert("print", printObj);
        arr.append(obj);
    }

    SmearManager::GetInstance()->mSlideStore->addPrintOnlyRequest(1, arr);
#endif

#if 0
    QJsonObject obj;
    obj.insert("stainBox", "right");

    QJsonObject slideObj;
    slideObj.insert("pos", 11);
    slideObj.insert("simple_id", "234");

    QJsonArray arr;
    arr.append(slideObj);
    obj.insert("slides", arr);

    StainManager::GetInstance()->mPools->startStainOnlyProcess(60, obj);

    QJsonObject obj1;
    obj1.insert("stainBox", "left");

    QJsonObject slideObj1;
    slideObj1.insert("pos", 5);
    slideObj1.insert("simple_id", "24");

    QJsonArray arr1;
    arr1.append(slideObj1);
    obj1.insert("slides", arr1);

    StainManager::GetInstance()->mPools->startStainOnlyProcess(61, obj1);
#endif
}


void DeviceSettingPage::on_pushButton_test_clicked()
{
//    MProcessManager::GetInstance()->startProcess("removeRemainStainSlides", 1);
    MProcessManager::GetInstance()->startProcess("shutDown", 1);
}

