#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "servers/log/f_log_server.h"
#include "messagecenter/f_message_center.h"
#include "f_common.h"

#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(FCommon::appName() + " - " + FCommon::appVersion());

    ui->page_process->configDeviceList();

    connect(ui->menu_function, &QMenu::triggered, this, &MainWindow::menuTriggered_slot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::menuTriggered_slot(QAction *action)
{
    if (action == ui->action_debug) {
        ui->stackedWidget->setCurrentWidget(ui->page_debug);
    } else if (action == ui->action_process) {
        ui->stackedWidget->setCurrentWidget(ui->page_process);
    }
}

void MainWindow::on_pushButton_process_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_process);
}

void MainWindow::on_pushButton_sensor_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_sensor);
}

void MainWindow::on_pushButton_debug_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_debug);
}

void MainWindow::close_slot()
{
    qDebug() << "RtBloodSmear is closing...";
    QTimer::singleShot(200, qApp, SLOT(quit()));
}

