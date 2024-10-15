#include "funciont_value_item.h"
#include "ui_funciont_value_item.h"

#include <QJsonObject>
#include <QJsonArray>

FunciontValueItem::FunciontValueItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunciontValueItem)
{
    ui->setupUi(this);
    setArgType(Arg_None);
}

FunciontValueItem::~FunciontValueItem()
{
    delete ui;
}

void FunciontValueItem::setApi(const QString &api)
{
    ui->pushButton->setText(api);
}

void FunciontValueItem::setArgType(ArgumentType type)
{
    m_argType = type;

    if (m_argType == Arg_None) {
        ui->spinBox_1_1->setVisible(false);
        ui->stackedWidget->setCurrentWidget(ui->page_int_1);
    } else if (m_argType == Arg_Integer_1) {
        ui->spinBox_1_1->setVisible(true);
        ui->stackedWidget->setCurrentWidget(ui->page_int_1);
    } else if (m_argType == Arg_Integer_2) {
        ui->stackedWidget->setCurrentWidget(ui->page_int_2);
    } else if (m_argType == Arg_Integer_3) {
        ui->stackedWidget->setCurrentWidget(ui->page_int_3);
    } else if (m_argType == Arg_Integer_4) {
        ui->stackedWidget->setCurrentWidget(ui->page_int_4);
    } else if (m_argType == Arg_String) {
        ui->stackedWidget->setCurrentWidget(ui->page_string);
    } else if (m_argType == Arg_Int_String) {
        ui->stackedWidget->setCurrentWidget(ui->page_int_string);
    }
}

void FunciontValueItem::on_pushButton_clicked()
{
    QJsonArray arr;

    if (m_argType == Arg_Integer_1) {
        arr.append(ui->spinBox_1_1->text().toInt());
    } else if (m_argType == Arg_Integer_2) {
        arr.append(ui->spinBox_2_1->text().toInt());
        arr.append(ui->spinBox_2_2->text().toInt());
    } else if (m_argType == Arg_Integer_3) {
        arr.append(ui->spinBox_3_1->text().toInt());
        arr.append(ui->spinBox_3_2->text().toInt());
        arr.append(ui->spinBox_3_3->text().toInt());
    } else if (m_argType == Arg_Integer_4) {
        arr.append(ui->spinBox_4_1->text().toInt());
        arr.append(ui->spinBox_4_2->text().toInt());
        arr.append(ui->spinBox_4_3->text().toInt());
        arr.append(ui->spinBox_4_4->text().toInt());
    } else if (m_argType == Arg_String) {
        arr.append(ui->lineEdit_2->text());
    } else if (m_argType == Arg_Int_String) {
        arr.append(ui->spinBox->text().toInt());
        arr.append(ui->lineEdit->text());
    }
    emit onFunctionExe_signal(ui->pushButton->text(), arr);
}

