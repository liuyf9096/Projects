#ifndef DOBOTLINKMAIN_H
#define DOBOTLINKMAIN_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QJsonObject>
#include <QProcess>
#include <QMap>

namespace Ui {
class DobotLinkMain;
}

class DPluginInterface;
class DAboutUsDialog;
class DLocalMonitorForm;
class DRemoteMonitorForm;
class DUpgradeDialog;
class DobotLinkMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit DobotLinkMain(QWidget *parent = nullptr);
    ~DobotLinkMain();

public slots:
    void showTrayMessageBox_slot(QString title, QString content);
    void showDobotLinkWidget_slot(QString widgetName, QJsonObject params);

signals:
    void onMainWidgetClose_signal();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::DobotLinkMain *ui;
    QSystemTrayIcon *m_SystemTray;
    QMap<QString, QAction*> m_pluginActionMap;

    DPluginInterface *interface = nullptr;

    DAboutUsDialog *m_aboutUsDialog = nullptr;
    DLocalMonitorForm *m_monitorWidget = nullptr;
    DRemoteMonitorForm *m_remoteMonitorForm = nullptr;
    DUpgradeDialog *m_upgradeDialog = nullptr;

    QProcess *m_process;

    void _systemTrayInit();
    bool _checkStartAtPowerOn();
    void _startMaintenanceProgress();

    void handleSerialportClose();

private slots:
    /* Action slot */
    void showOnTop_slot();
    void startLogAct_slot();
    void openLogFileAct_slot();
    void startAtPowerOnAct_slot();
    void helpActionOnAct_slot();
    void upgradeActionAct_slot();
    void showAboutUsAct_slot();
    void exitApp_slot();

    void onMenuTriggered_slot(QAction *action);
    void systemTrayActivated_slot(QSystemTrayIcon::ActivationReason reason);
    void handleShowLogMsg_slot(QString message);

    /* Button slot */
    void on_btn_clearbox_clicked();
    void on_checkBox_print_stateChanged(int arg1);
};

#endif // DOBOTLINKMAIN_H
