#ifndef CHANGE_DECRIPTION_DIALOG_H
#define CHANGE_DECRIPTION_DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class ChangeDecripionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeDecripionDialog(QWidget *parent = nullptr);
    ~ChangeDecripionDialog();

signals:
    void sendMessage_signal(const QString &text);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
};

#endif // CHANGE_DECRIPTION_DIALOG_H
