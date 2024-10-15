#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "f_ws_server.h"

#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_wsServer = new FWebSocketServer(this);

//    m_wsServer->checkAlive(true, 10);

    connect(m_wsServer, &FWebSocketServer::onClientConnected_signal, [=](const QString &client_id){
        m_clientId = client_id;
        qDebug() << "Client Connected." << client_id;
    });

    connect(m_wsServer, &FWebSocketServer::onSendMsgError_signal, [=](const QString &client_id, const QString &message){
        if (m_clientId == client_id) {
            m_clientId.clear();
        }
        qDebug() << "Send Msg Error." << client_id << message;
    });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimer_slot);
//    m_timer->start(200);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    static int id = 0;

    JPacket p(PacketType::Request, id++);
    p.device = "Server";
    p.module = "Test";
    p.api = "AutoTest";

    m_wsServer->sendPacket(m_clientId, p);
}

void MainWindow::onTimer_slot()
{
    if (m_clientId.isEmpty() == false) {
        on_pushButton_clicked();
    }
}

void MainWindow::on_pushButton_open_clicked()
{
    quint16 port = ui->lineEdit_port->text().toUInt();
    m_wsServer->listen(port, "Test");
}

void MainWindow::on_pushButton_close_clicked()
{
    m_wsServer->closeServer();
}

