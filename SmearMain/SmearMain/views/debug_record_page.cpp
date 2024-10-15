#include "debug_record_page.h"
#include "ui_debug_record_page.h"

#include "servers/log/f_log_server.h"

#include "track/track_manager.h"
#include "smear/smear_manager.h"
#include "stain/stain_manager.h"

#include <QDebug>

DebugRecordPage::DebugRecordPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugRecordPage)
{
    ui->setupUi(this);

    auto log = FLogServer::GetInstance();
    connect(log, &FLogServer::logMessage_signal,
            this, &DebugRecordPage::handleDebugMessage_slot);

//    configTabs();
}

DebugRecordPage::~DebugRecordPage()
{
    delete ui;
}

void DebugRecordPage::configTabs()
{
    for (auto & mid : TrackManager::GetInstance()->getModuleList()) {
        addTab(mid);
    }
    for (auto & mid : SmearManager::GetInstance()->getModuleList()) {
        addTab(mid);
    }
    for (auto & mid : StainManager::GetInstance()->getModuleList()) {
        addTab(mid);
    }
}

void DebugRecordPage::addTab(const QString &name)
{
    QWidget *widget = new QWidget(ui->tabWidget);
    widget->setObjectName("widget_" + name);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    layout->setObjectName(name + "_layout");

    QPlainTextEdit *plainTextEdit = new QPlainTextEdit(widget);
    plainTextEdit->setObjectName("plainTextEdit_" + name);
    plainTextEdit->setMaximumBlockCount(1000);

    layout->addWidget(plainTextEdit);

    ui->tabWidget->addTab(widget, name);
    m_deviceTextMap.insert(name, plainTextEdit);
}

void DebugRecordPage::handleDebugMessage_slot(const QJsonObject &obj)
{
    QString message = obj.value("message").toString();
    QString type = obj.value("type").toString();
//    QString time = obj.value("time").toString();
    QString msg = QString("[%1]%2").arg(type, message);
    ui->plainTextEdit_total->appendPlainText(msg);

    if (type != "debug") {
        ui->plainTextEdit_warning->appendPlainText(msg);
    }
    if (message.startsWith("[P]")) {
        message.remove("[P]");

        QString dev_id = getDeviceType(message);
        auto plainTextEdit = m_deviceTextMap.value(dev_id);
        plainTextEdit->appendPlainText(message);
    }
}

QString DebugRecordPage::getDeviceType(const QString &message)
{
    QRegExp sep("\\{\\w*\\}");
    int pos = sep.indexIn(message);
    if (pos > -1) {
        QString type = sep.cap();
        type.remove("{");
        type.remove("}");
        return type;
    }
    return QString();
}
