#include "PluginItemForm.h"
#include "ui_PluginItemForm.h"

#include <QDebug>

PluginItemForm::PluginItemForm(QString pluginName, QWidget *parent) :
    QWidget(parent),
    pluginName(pluginName),
    ui(new Ui::PluginItemForm)
{
    ui->setupUi(this);

    ui->label_name->setText(pluginName);
    ui->stackedWidget->setCurrentWidget(ui->page_enquiry);
}

PluginItemForm::~PluginItemForm()
{
    delete ui;
}

void PluginItemForm::setTitle(QString name)
{
    ui->label_name->setText(name);
}

void PluginItemForm::setLocalVersion(QString version)
{
    ui->label_local->setText(version);
}

void PluginItemForm::setLatestVersion(QString version)
{
    ui->label_new->setText(version);
}

void PluginItemForm::setInfoState(PluginItemForm::InfoState state)
{
    switch (state) {    
    case ENQUIRY:
        ui->stackedWidget->setCurrentWidget(ui->page_nonew);
        break;
    case NONEW:
        ui->stackedWidget->setCurrentWidget(ui->page_nonew);
        break;
    case NEW_VERSION:
        ui->stackedWidget->setCurrentWidget(ui->page_start);
        break;
    case DOWNLOADING:
        ui->stackedWidget->setCurrentWidget(ui->page_download);
        break;
    case FINISH:
        ui->stackedWidget->setCurrentWidget(ui->page_finishdownload);
        break;
    }
}

void PluginItemForm::setProgressBar(int value)
{
    ui->progressBar->setValue(value);
}

void PluginItemForm::on_pushButton_update_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_download);
    emit onStartUpdate_signal(pluginName);
}

void PluginItemForm::on_pushButton_cancel_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_start);
    emit onCancelUpdate_signal(pluginName);
}
