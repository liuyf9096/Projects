#include "change_decription_dialog.h"
#include "ui_change_decription_dialog.h"

ChangeDecripionDialog::ChangeDecripionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

ChangeDecripionDialog::~ChangeDecripionDialog()
{
    delete ui;
}

void ChangeDecripionDialog::on_pushButton_clicked()
{
    if (ui->lineEdit_ip->text().isEmpty() == false) {
        QString str = QString("ChangeDecription,%1,%2")
                .arg(ui->lineEdit_ip->text(), ui->lineEdit_decription->text());
        emit sendMessage_signal(str);
    }
}

