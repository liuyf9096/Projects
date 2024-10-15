#include "debug_command_dialog.h"
#include "ui_debug_command_dialog.h"
#include "funciont_value_item.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QJsonArray>
#include <QDebug>

DebugCommandDialog::DebugCommandDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugCommandDialog)
{
    ui->setupUi(this);
}

DebugCommandDialog::~DebugCommandDialog()
{
    delete ui;
}

void DebugCommandDialog::setTitle(const QString &title)
{
    this->setWindowTitle(title);
}

void DebugCommandDialog::addFunctionList(const QStringList &list, int argCount)
{
    for (int i = 0; i < list.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(0, 46));
        item->setData(Qt::UserRole, list.at(i));

        FunciontValueItem *funcItem = new FunciontValueItem();
        funcItem->setApi(list.at(i));
        funcItem->setArgCount(argCount);

        connect(funcItem, &FunciontValueItem::onFunctionClicked_signal,
                this, [=](const QString &api, int count, int arg1, int arg2, int arg3)
        {
            QJsonArray argArr;
            if (count >= 1) {
                argArr.append(arg1);
            }
            if (count >= 2) {
                argArr.append(arg2);
            }
            if (count >= 3) {
                argArr.append(arg3);
            }
            emit sendCommand_signal(api, argArr);
        });

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, funcItem);
    }
}

