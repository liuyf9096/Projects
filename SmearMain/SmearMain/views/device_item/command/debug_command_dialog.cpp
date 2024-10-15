#include "debug_command_dialog.h"
#include "ui_debug_command_dialog.h"

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

void DebugCommandDialog::addFunctionList(const QStringList &list, ArgumentType type)
{
    for (int i = 0; i < list.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(0, 46));
        item->setData(Qt::UserRole, list.at(i));

        FunciontValueItem *funcItem = new FunciontValueItem();
        funcItem->setArgType(type);
        funcItem->setApi(list.at(i));

        connect(funcItem, &FunciontValueItem::onFunctionExe_signal,
                this, &DebugCommandDialog::sendCommand_signal);

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, funcItem);
    }
}


