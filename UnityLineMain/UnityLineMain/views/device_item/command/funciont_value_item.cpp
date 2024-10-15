#include "funciont_value_item.h"
#include "ui_funciont_value_item.h"

#include <QDebug>

FunciontValueItem::FunciontValueItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunciontValueItem)
{
    ui->setupUi(this);
}

FunciontValueItem::~FunciontValueItem()
{
    delete ui;
}

void FunciontValueItem::setApi(const QString &api)
{
    ui->pushButton->setText(api);
}

void FunciontValueItem::setArgCount(int count)
{
    m_argCount = count;

    if (count == 0) {
        ui->spinBox_1->hide();
        ui->spinBox_2->hide();
        ui->spinBox_1->setEnabled(false);
        ui->spinBox_2->setEnabled(false);
    } else if (count == 1) {
        ui->spinBox_1->show();
        ui->spinBox_2->show();
        ui->spinBox_1->setEnabled(true);
        ui->spinBox_2->setEnabled(false);
    } else if (count == 2) {
        ui->spinBox_1->show();
        ui->spinBox_2->show();
        ui->spinBox_1->setEnabled(true);
        ui->spinBox_2->setEnabled(true);
    } else {
        qWarning() << "Arg count error." << count;
    }
}

void FunciontValueItem::on_pushButton_clicked()
{
    int arg1 = ui->spinBox_1->text().toInt();
    int arg2 = ui->spinBox_2->text().toInt();

    emit onFunctionClicked_signal(ui->pushButton->text(), m_argCount, arg1, arg2);
}

