#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "f_settings.h"
#include "f_common.h"
#include "change_decription_dialog.h"

#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString title = QString("%1 v%2").arg(FCommon::appName(), FCommon::appVersion());
    setWindowTitle(title);

    addMessage(FCommon::printSystemInfo());

    m_udp = new QUdpSocket(this);
    connect(m_udp, &QUdpSocket::readyRead,
            this, &MainWindow::onReadDatagram_slot);

    m_dialog = new ChangeDecripionDialog(this);
    connect(m_dialog, &ChangeDecripionDialog::sendMessage_signal,
            this, &MainWindow::sendMessage_slot);

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Host Name"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Ip Address"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("Description"));
    ui->tableView->setModel(m_model);
    ui->tableView->setItemDelegateForColumn(1, new RCenterDelegate(this));

    m_bindPort = FSettings::GetInstance()->getListenPort();
    m_sendPort = FSettings::GetInstance()->getSendPort();
    addMessage(QString("read config file... listen port:%1, send port:%2.").arg(m_bindPort).arg(m_sendPort));

    bool ok = m_udp->bind(m_bindPort, QUdpSocket::ShareAddress);
    if (ok) {
        addMessage(QString("bind port %1... success.").arg(m_bindPort));
    } else {
        addMessage(QString("bind port %1 false, please try later.").arg(m_bindPort));
    }
    addMessage("please do not open VMware Network Adapter VMnet!");
    addMessage("");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_send_clicked()
{
    QString message = ui->lineEdit_send->text();
    m_model->removeRows(0, m_model->rowCount());
    sendUdpMessage(message);
}

void MainWindow::sendMessage_slot(const QString &message)
{
    sendUdpMessage(message);
}

bool MainWindow::sendUdpMessage(const QString &message)
{
    /* please do not open VMware Network Adapter VMnet */
    qint64 count = m_udp->writeDatagram(message.toUtf8(), QHostAddress::Broadcast, m_sendPort);
    if (count > 0) {
        addMessage(QString("<< [UDP:%1] %2").arg(m_sendPort).arg(message));
        ui->statusBar->clearMessage();
        return true;
    } else {
        ui->statusBar->showMessage(m_udp->errorString());
    }
    return false;
}

void MainWindow::onReadDatagram_slot()
{
    while (m_udp->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udp->receiveDatagram();
        QString message(datagram.data());
        if (!message.isEmpty()) {
            addMessage(QString(">> [UDP:%1] %2").arg(m_bindPort).arg(message));
            updateForm(message);
        }
    }
}

void MainWindow::updateForm(const QString &message)
{
    QList<QStandardItem *> itemlist;
    QStringList list = message.split(",");
    if (list.count() < 3) {
        return;
    }
    QString hostName = list.at(0);
    QString ip = list.at(1);
    QString description = list.at(2);

    if (!hostName.isEmpty()) {
        auto item = new QStandardItem(hostName);
        item->setEditable(false);
        item->setTextAlignment(Qt::AlignCenter);
        itemlist.append(item);
    }
    if (!ip.isEmpty()) {
        auto item = new QStandardItem(ip);
        item->setTextAlignment(Qt::AlignCenter);
        item->setEditable(true);
        itemlist.append(item);
    }
    if (!description.isEmpty()) {
        auto item = new QStandardItem(description);
        item->setTextAlignment(Qt::AlignCenter);
        item->setEditable(false);
        itemlist.append(item);
    }

    QList<QStandardItem*> itmeList_0 = m_model->findItems(ip, Qt::MatchExactly, 1);
    if (itmeList_0.isEmpty()) {
        m_model->appendRow(itemlist);
    } else {
        m_model->removeRow(itmeList_0.first()->row());
        m_model->appendRow(itemlist);
    }
}

void MainWindow::addMessage(const QString &message)
{
    ui->plainTextEdit->appendPlainText(message);
}

void MainWindow::on_pushButton_clicked()
{
    static int count = 0;
    count++;
    if (count >= 3) {
        count = 0;
        m_dialog->show();
    }
}

RCenterDelegate::RCenterDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *RCenterDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QLineEdit *edit = new QLineEdit(parent);
    edit->setAlignment(Qt::AlignCenter);
    return edit;
}
