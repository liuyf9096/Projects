#ifndef DEBUG_COMMAND_DIALOG_H
#define DEBUG_COMMAND_DIALOG_H

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
    void addFunctionList(const QStringList &list, int argCount);

signals:
    void sendCommand_signal(const QString &api, const QJsonValue &arg = QJsonValue());

private:
    Ui::DebugCommandDialog *ui;
};

#endif // DEBUG_COMMAND_DIALOG_H
