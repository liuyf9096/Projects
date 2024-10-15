#ifndef DEBUG_COMMAND_DIALOG_H
#define DEBUG_COMMAND_DIALOG_H

#include "funciont_value_item.h"

#include <QDialog>
#include <QJsonValue>
#include <QListWidgetItem>

namespace Ui {
class DebugCommandDialog;
}

class DebugCommandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugCommandDialog(QWidget *parent = nullptr);
    ~DebugCommandDialog();

    void setTitle(const QString &title);
    void addFunctionList(const QStringList &list, ArgumentType type);

signals:
    void sendCommand_signal(const QString &api, const QJsonValue &arg = QJsonValue());

private:
    Ui::DebugCommandDialog *ui;

private slots:
//    void onFunctionExe_slot(const QString &api, const QJsonValue &arg);
};

#endif // DEBUG_COMMAND_DIALOG_H
