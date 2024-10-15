#include "widget.h"

#include "include/f_ws_client.h"
#include "include/f_jsonrpc_parser.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_client = new FWsClient(this);
    m_client->setClientName("Demo");
    connect(m_client, &FWsClient::onConnected_signal, this, [=](){
        qDebug() << "connected.";
    });
    connect(m_client, &FWsClient::onDisConnected_signal, this, [=](){
        qDebug() << "disconnected.";
    });
    connect(m_client, &FWsClient::onReceiveMessagePacket_signal, this, [=](const JPacket &packet){
        qDebug() << "receive" << packet;
    });
    connect(m_client, &FWsClient::onNewServerAddress_signal, this, [&](const QString &address, quint16 port){
        qDebug() << address << port;
    });
}

Widget::~Widget()
{
    m_client->disconnectServer();
    delete m_client;
}

void Widget::connectServer(const QString &ip, quint16 port)
{
    m_client->autoDetectServer("DMU");
    m_client->connectServer(ip, port, "DMU");
//    m_client->checkAlive(true, 3);
}

void Widget::sendTestMessage()
{
    static int id = 0;

    JPacket p(PacketType::Request, id++);
    p.device = "NoOne";
    p.module = "Test";
    p.api = "AutoTest";

    m_client->sendMessage(p);
}

