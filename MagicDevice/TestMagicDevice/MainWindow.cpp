#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    id = 0;
    ui->setupUi(this);

    device = new MagicDevice(DEVICE_MAGICIAN, this);
//    device = new MagicDevice(DEVICE_M1, this);
    device->setDeviceType(DEVICE_M1);
//    device->setPortName("COM3");
    device->setPortName("192.168.53.74");

    connect(device, &MagicDevice::onResultMessage_signal, this,
            [=](quint64 id, QString cmd, int res = 0, QJsonValue params = QJsonValue()){
        qDebug().noquote() << QString("[receive id:%1] cmd:%2 res:%3").arg(id).arg(cmd).arg(res)
                           << params;
    });

//    qDebug() << device->getCommandList();
    qDebug() << sizeof (float);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_search_clicked()
{
    QStringList list = device->getDeviceList(false);
    qDebug() << list;
}

void MainWindow::on_pushButton_connect_clicked()
{
    device->connectDevice(1);
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    device->disConnectDevice(2);
}

void MainWindow::on_pushButton_send_clicked()
{ 
#if 0
    QJsonObject obj;
    obj.insert("isWithL", true);
    obj.insert("version", 2);

    MessagePacket p(1, "SetDeviceWithL");
    p.setPacket(false, obj);
#elif 0
    MessagePacket p("SetHOMECmd", 1);
    p.setIsQueue(true);
    p.setWaitForFinishEnable(true);
#endif
    MessagePacket p1("GetPose", id++);
    p1.setTargetType(DEVICE_M1);
    device->sendCommand(p1);
}

void MainWindow::on_pushButton_send2_clicked()
{
    MessagePacket p("GetPose", id);
    MessagePacket p1("SetRCmd", id);
    QJsonObject paramsObj;
    paramsObj.insert("r", 200);
    p1.setParams(paramsObj);

    p1.setIsQueue(true);
    p1.setWaitForFinishEnable(true, 5000);

    MessagePacket p2("GetDeviceSN", id);

    DPacketList list;
    list << p << p1;
    device->sendCommandList(id++, list);
}

void MainWindow::on_pushButton_send3_clicked()
{
    MessagePacket p("SetHOMEParams", id++);

    QJsonArray arr1, arr2;
    arr1 << 15 << 16 << 17 << 30;
    arr2 << 50 << 51 << 52 << 53;

    QJsonObject parmas;
    parmas.insert("x", 200);
    parmas.insert("y", 0);
    parmas.insert("z", 0);
    parmas.insert("r", 0);

    if (!parmas.isEmpty()) {
        p.setParams(parmas);
    }

//    p.setIsQueue(false);
//    p.setWaitForFinishEnable(true, 5000, 20000);
    device->sendCommand(p);
}

void MainWindow::on_pushButton_send4_clicked()
{
    MessagePacket p("GetHOMEParams", id++);

    QJsonObject parmas;
//    parmas.insert("address", 1);
    if (!parmas.isEmpty()) {
        p.setParams(parmas);
    }
    device->sendCommand(p, false);
}

void MainWindow::on_pushButton_m1_clicked()
{
//    MessagePacket p("GetHOMEParams", id++);

//    QJsonObject parmas;
////    parmas.insert("address", 1);
//    if (!parmas.isEmpty()) {
//        p.setParams(parmas);
//    }
//    device->sendCommand(p, false);
}
