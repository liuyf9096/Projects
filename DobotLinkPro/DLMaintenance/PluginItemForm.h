#ifndef PLUGINITEMFORM_H
#define PLUGINITEMFORM_H

#include <QWidget>

namespace Ui {
class PluginItemForm;
}

class PluginItemForm : public QWidget
{
    Q_OBJECT

public:
    enum InfoState {ENQUIRY, NONEW, NEW_VERSION, DOWNLOADING, FINISH};
    explicit PluginItemForm(QString pluginName, QWidget *parent = nullptr);
    ~PluginItemForm();

    const QString pluginName;

    void setTitle(QString name);
    void setLocalVersion(QString version);
    void setLatestVersion(QString version);
    void setInfoState(InfoState state);
    void setProgressBar(int value);

signals:
    void onStartUpdate_signal(QString pluginName);
    void onCancelUpdate_signal(QString pluginName);

private slots:
    void on_pushButton_update_clicked();

    void on_pushButton_cancel_clicked();

private:
    Ui::PluginItemForm *ui;
};

#endif // PLUGINITEMFORM_H
