#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "include/f_ws_client.h"
#include "include/f_jsonrpc_parser.h"

#include <QTimer>
#include <QThread>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_id(0)
{
    ui->setupUi(this);

    mTimer = new QTimer(this);
    connect(mTimer, &QTimer::timeout, [=](){
        on_pushButton_sendobj_clicked();
    });

    mClient = new FWsClient();
    connect(mClient, &FWsClient::onConnected_signal, this, [=](){
        qDebug() << "connected.";
    });
    connect(mClient, &FWsClient::onDisConnected_signal, this, [](){
        qDebug() << "disconnected.";
    });
    connect(mClient, &FWsClient::onReceiveMessagePacket_signal,
            this, &MainWindow::onReceiveMessagePacket_slot);
    connect(mClient, &FWsClient::onReceiveMessage_signal,
            this, &MainWindow::onReceiveMessage_slot);

    m_tread = new QThread();
    connect(m_tread, &QThread::finished, m_tread, &QThread::deleteLater);
    mClient->moveToThread(m_tread);
    m_tread->start();
}

MainWindow::~MainWindow()
{
    m_tread->quit();
    m_tread->wait(500);
    if (m_tread->isFinished()) {
        qInfo() << "thread quit success.";
    } else {
        qWarning() << "thread can not quit. timeout";
    }
    delete mClient;
    delete m_tread;
    delete ui;
}

void MainWindow::on_pushButton_connect_clicked()
{
    mClient->connectServer(ui->lineEdit_address->text(), ui->lineEdit_port->text().toInt());
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    mClient->disconnectServer();
}

void MainWindow::on_pushButton_delete_clicked()
{
    delete mClient;
}

void MainWindow::on_pushButton_starttest_clicked()
{
    mTimer->start(ui->spinBox_interval->text().toInt());
}

void MainWindow::on_pushButton_stoptest_clicked()
{
    mTimer->stop();
}

void MainWindow::on_pushButton_sendobj_clicked()
{
    JPacket p(PacketType::Request, m_id++);
    p.device = "BD2000";
    p.module = "SmearTrack";
    p.api = "ResetAll";

    QJsonObject obj;
    obj.insert("code", 200);
    p.paramsValue = obj;

    mClient->sendMessage(p);
}

void MainWindow::on_pushButton_sendtext_clicked()
{
    mClient->sendMessage(ui->lineEdit_text->text());
}

void MainWindow::onReceiveMessagePacket_slot(const JPacket &packet)
{
    qDebug() << "debug" << packet;
}

void MainWindow::onReceiveMessage_slot(const QString &message)
{
    ui->plainTextEdit->appendPlainText(message);

    QScrollBar *scrollBar = ui->plainTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
