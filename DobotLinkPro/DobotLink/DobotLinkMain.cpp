#include "DobotLinkMain.h"
#include "ui_DobotLinkMain.h"

#include <QSettings>
#include <QMessageBox>
#include <QScrollBar>
#include <QDir>
#include <QLocale>
#include <QDate>
#include <QDir>
#include <QTimer>
#include <QDebug>

#include "DPluginInterface.h"
#include "MessageCenter/DMessageCenter.h"
#include "Module/DNetworkManager.h"
#include "Module/DLogger.h"
#include "Module/DOpenFile.h"
#include "Module/DSettings.h"
#include "Views/DAboutUsDialog.h"
#include "Views/DLocalMonitorForm.h"
#include "Views/DRemoteMonitorForm.h"
#include "Views/DUpgradeDialog.h"

const QString VERSION("5.7.0");

const QString MACHINE_REG_RUN("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
const QString USER_REG_RUN("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");

DobotLinkMain::DobotLinkMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::DobotLinkMain)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    ui->actionCamera->setEnabled(false);
    ui->actionRemoteMonitor->setEnabled(false);
#endif

    setWindowTitle("DobotLink V" + VERSION);
    setWindowIcon(QIcon("://images/DobotLinkIcon.ico"));

    /* systemtray init */
    _systemTrayInit();

    /* set plugin list */
    m_pluginActionMap.insert(ui->actionMagicDevicePlugin->text(), ui->actionMagicDevicePlugin);
    m_pluginActionMap.insert(ui->actionIndustrialRobotPlugin->text(), ui->actionIndustrialRobotPlugin);
    m_pluginActionMap.insert(ui->actionArduinoPlugin->text(), ui->actionArduinoPlugin);
    m_pluginActionMap.insert(ui->actionMicrobitPlugin->text(), ui->actionMicrobitPlugin);
    m_pluginActionMap.insert(ui->actionDownloadPlugin->text(), ui->actionDownloadPlugin);

    ui->stackedWidget->setCurrentWidget(ui->page_Monitor);

    connect(ui->menuMonitor, &QMenu::triggered, this, &DobotLinkMain::onMenuTriggered_slot);
    connect(ui->menuTools, &QMenu::triggered, this, &DobotLinkMain::onMenuTriggered_slot);
    connect(ui->menuPlugins, &QMenu::triggered, this, &DobotLinkMain::onMenuTriggered_slot);

    /* views init */
    connect(this, &DobotLinkMain::onMainWidgetClose_signal,
            ui->page_SerialPort, &DSerialTestForm::onWidgetClose_slot);

    m_aboutUsDialog = new DAboutUsDialog(this);
    m_aboutUsDialog->hide();

    m_monitorWidget = new DLocalMonitorForm();
    m_remoteMonitorForm = new DRemoteMonitorForm();
    m_upgradeDialog = new DUpgradeDialog(this);
    connect(m_upgradeDialog, &DUpgradeDialog::closeApp_signal,
            this, &DobotLinkMain::exitApp_slot);

    /* log init */
    DLogger *logger = DLogger::getInstance();
    connect(logger, &DLogger::logMessage_signal,
            this, &DobotLinkMain::handleShowLogMsg_slot);

//! [user init here]
    qDebug().noquote() << "DobotLink Version:" << VERSION;

    DNetworkManager::getInstance()->showIPAddress();

    /* config settings init */
    DSettings::getInstance()->setVersion("main", VERSION);

    /* message center init */
    DMessageCenter *msgCenter = DMessageCenter::getInstance();

    connect(msgCenter, &DMessageCenter::showTrayMessage_signal,
            this, &DobotLinkMain::showTrayMessageBox_slot);
    connect(msgCenter, &DMessageCenter::showDobotLinkWidget_signal,
            this, &DobotLinkMain::showDobotLinkWidget_slot);
    connect(msgCenter, &DMessageCenter::onLoadPluginFinish_signal, this, [=](QString name, bool isLoaded){
        QAction *action = m_pluginActionMap.value(name);
        if (action) {
            action->setChecked(isLoaded);
        }
    });
    msgCenter->loadDefaultPlugin();

    m_process = new QProcess(this);
}

DobotLinkMain::~DobotLinkMain()
{
    delete m_monitorWidget;
    delete m_remoteMonitorForm;

    delete ui;
    qDebug() << "DobotLinkMain already closed. See you next time.";
}

void DobotLinkMain::_systemTrayInit()
{
    m_SystemTray = new QSystemTrayIcon(this);
    QMenu *menu = new QMenu(this);
    QAction *showOnTop = new QAction(tr("Show on top"), menu);
    QMenu *logMenu = new QMenu(tr("Log"), menu);
    QAction *startLogAction = new QAction(tr("Start Log"), logMenu);
    QAction *openLogFileAction = new QAction(tr("Open Log File"), logMenu);
    QAction *startAtPowerOnAction = new QAction(tr("Start With System"), menu);
    QAction *helpAction = new QAction(tr("Help"), menu);
    QAction *upgradeAction = new QAction(tr("check upgrade"), menu);
    QAction *aboutUsAction = new QAction(tr("About us"), menu);
    QAction *exitAction = new QAction(tr("Exit"), menu);

//! [1] init Actions
    showOnTop->setCheckable(true);
    startAtPowerOnAction->setCheckable(true);
    startLogAction->setCheckable(true);

    bool isShowOnTop = DSettings::getInstance()->getShowTopOn();
    if (isShowOnTop) {
        setWindowFlags(Qt::WindowStaysOnTopHint);
    }
    showOnTop->setChecked(isShowOnTop);

    bool isLogging = DSettings::getInstance()->getIsLogging();
    if (isLogging) {
        startLogAction->setChecked(true);
        DLogger::getInstance()->startLogging();
    }

    bool isPrintable = DSettings::getInstance()->getIsPrintable();
    if (isPrintable) {
        ui->checkBox_print->setChecked(true);
    } else {
        ui->checkBox_print->setChecked(false);
    }

#ifdef Q_OS_WIN
    if (_checkStartAtPowerOn() == true) {
        startAtPowerOnAction->setChecked(true);
    }
#endif

    connect(showOnTop, &QAction::triggered, this, &DobotLinkMain::showOnTop_slot);
    connect(startLogAction, &QAction::triggered, this, &DobotLinkMain::startLogAct_slot);
    connect(openLogFileAction, &QAction::triggered, this, &DobotLinkMain::openLogFileAct_slot);
    connect(startAtPowerOnAction, &QAction::triggered, this, &DobotLinkMain::startAtPowerOnAct_slot);
    connect(helpAction, &QAction::triggered, this, &DobotLinkMain::helpActionOnAct_slot);
    connect(upgradeAction, &QAction::triggered, this, &DobotLinkMain::upgradeActionAct_slot);
    connect(aboutUsAction, &QAction::triggered, this, &DobotLinkMain::showAboutUsAct_slot);
    connect(exitAction, &QAction::triggered, this, &DobotLinkMain::exitApp_slot);

//! [2] init Menu
    menu->addAction(showOnTop);
    menu->addMenu(logMenu);
    logMenu->addAction(startLogAction);
    logMenu->addAction(openLogFileAction);
#ifdef Q_OS_WIN
    menu->addAction(startAtPowerOnAction);
#endif
    menu->addAction(helpAction);
    menu->addAction(upgradeAction);
    menu->addAction(aboutUsAction);
    menu->addSeparator();
    menu->addAction(exitAction);

//! [3] init SystemTray
    m_SystemTray->setContextMenu(menu);
    m_SystemTray->setIcon(QIcon("://images/DobotLinkIcon.ico"));
    m_SystemTray->setToolTip("DobotLink");

    if (DSettings::getInstance()->getShowSystemTray() == true) {
        m_SystemTray->show();
    }

    connect(m_SystemTray, &QSystemTrayIcon::activated,
            this, &DobotLinkMain::systemTrayActivated_slot);

    m_SystemTray->showMessage(QString(), tr("DobotLink is Running."),
                              QSystemTrayIcon::MessageIcon::NoIcon, 500);
}

/* check power-on partern */
bool DobotLinkMain::_checkStartAtPowerOn()
{
    QSettings regSetting(USER_REG_RUN, QSettings::NativeFormat);
    QString appPathStr = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString pathStr = regSetting.value(QApplication::applicationName()).toString();
    if (pathStr.isEmpty()) {
        return false;
    }
    return true;
}

void DobotLinkMain::_startMaintenanceProgress()
{
    QString appPath = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    QDir dir(appPath);

#ifdef Q_OS_WIN
    QString program = dir.absoluteFilePath("DLMaintenance.exe");
#elif defined (Q_OS_MAC)
    QString program = dir.absoluteFilePath("DLMaintenance");
#endif
    qDebug() << "program:" << program;

    bool ok = m_process->startDetached(program, QStringList(program));
    if (ok) {
        qDebug().noquote() << tr("started DLMaintenance.exe program.");
    } else {
        qDebug() << "started failed.";
    }
}

/* widget show on the top */
void DobotLinkMain::showOnTop_slot()
{
    auto action = qobject_cast<QAction *>(sender());
    if (action) {
        if (action->isChecked() == true) {
            setWindowFlags(Qt::WindowStaysOnTopHint);
            DSettings::getInstance()->setShowTopOn(true);
            this->show();
            qDebug().noquote() << tr("open show-on-top");
        } else {
            setWindowFlags(nullptr);
            DSettings::getInstance()->setShowTopOn(false);
            this->show();
            qDebug().noquote() << tr("close show-on-top");
        }
    }
}

void DobotLinkMain::startLogAct_slot()
{
    auto action = qobject_cast<QAction *>(sender());
    if (action) {
        if (action->isChecked()) {
            DLogger::getInstance()->startLogging();
            DSettings::getInstance()->setIsLogging(true);
        } else {
            DLogger::getInstance()->stopLogging();
            DSettings::getInstance()->setIsLogging(false);
        }
    }
}

/* open log dir */
void DobotLinkMain::openLogFileAct_slot()
{
#if 1
    DLogger::getInstance()->openLogDir();
#else
    DLogger::getInstance()->openLogFile();
#endif
}

/* show message bubble */
void DobotLinkMain::showTrayMessageBox_slot(QString title, QString content)
{
    if (m_SystemTray->isVisible()) {
        m_SystemTray->showMessage(title, content, QSystemTrayIcon::MessageIcon::NoIcon, 500);
    }
}

/* show serialport tool */
void DobotLinkMain::showDobotLinkWidget_slot(QString widgetName, QJsonObject params)
{
    bool on = params.value("on").toBool();

    if (widgetName == "SerialPortTool") {
        if (on) {
            ui->stackedWidget->setCurrentWidget(ui->page_SerialPort);
            this->show();
            this->activateWindow();
            this->raise();
        } else {
            this->hide();
        }
    } else if (widgetName == "HelpMenu") {
        helpActionOnAct_slot();
    } else if (widgetName == "CommMonitor") {
        if (on) {
            ui->stackedWidget->setCurrentWidget(ui->page_Monitor);
            this->show();
            this->activateWindow();
            this->raise();
        } else {
            this->hide();
        }
    } else if (widgetName == "SystemTray") {
        if (params.value("on").toBool() == true) {
            m_SystemTray->show();
        } else {
            m_SystemTray->hide();
        }
    }
}

void DobotLinkMain::closeEvent(QCloseEvent *event)
{
    if (ui->stackedWidget->currentWidget() == ui->page_SerialPort) {
        if (ui->page_SerialPort->isSerialPortRunning() == true) {
            emit onMainWidgetClose_signal();
        }
    }
    event->accept();
}

/* set start dobotlink at power-on */
void DobotLinkMain::startAtPowerOnAct_slot()
{
    auto action = qobject_cast<QAction *>(sender());

    QSettings regSetting(USER_REG_RUN, QSettings::NativeFormat);
    QString appPathStr = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

    if (action->isChecked()) {
        regSetting.setValue(QApplication::applicationName(), appPathStr);
    } else {
        regSetting.remove(QApplication::applicationName());
    }
}

/* open help file */
void DobotLinkMain::helpActionOnAct_slot()
{
//    DOpenFile::getInstance()->openFileWithVSCode("protocolMenu.md");
#ifdef Q_OS_WIN
    DOpenFile::getInstance()->openFileWithDesktopServices("DobotlinkHelp.CHM");
#else
    DOpenFile::getInstance()->openFileWithDesktopServices("index.htm");
#endif
}

void DobotLinkMain::upgradeActionAct_slot()
{
    m_upgradeDialog->checkUpgrade();
    m_upgradeDialog->show();
}

/* show about us widget */
void DobotLinkMain::showAboutUsAct_slot()
{
    m_aboutUsDialog->setVersion(VERSION);

    QString dateStr = QString(__DATE__).replace("  ", " 0");
    QDate date = QLocale(QLocale::English).toDate(dateStr, "MMM dd yyyy");
    QString releaseDate = date.toString("yyyy/MM/dd");
    m_aboutUsDialog->setReleaseTime(releaseDate);

    m_aboutUsDialog->show();
}

/* exit slot */
void DobotLinkMain::exitApp_slot()
{
    auto action = qobject_cast<QAction *>(sender());
    if (action) {
        action->setEnabled(false);
    }

    qDebug() << "DobotLinkMain closing...";
    QTimer::singleShot(200, qApp, SLOT(quit()));

    DMessageCenter::getInstance()->onClose_slot();
}

/* click top menu slot */
void DobotLinkMain::onMenuTriggered_slot(QAction *action)
{
    auto *menu = qobject_cast<QMenu *>(sender());

    if (menu == ui->menuMonitor) {
        /* MONITOR */

        if (action == ui->actionMonitor) {
            ui->stackedWidget->setCurrentWidget(ui->page_Monitor);
        }
    } else if (menu == ui->menuTools) {
        /* TOOLS */

        if (action == ui->actionSerialPort) {
            ui->stackedWidget->setCurrentWidget(ui->page_SerialPort);
        } else if (action == ui->actionDeviceTest) {
            ui->stackedWidget->setCurrentWidget(ui->page_Magician);
        } else if (action == ui->actionArduinoTest) {
            ui->stackedWidget->setCurrentWidget(ui->page_Arduino);
        } else if (action == ui->actionMicrobitTest) {
            ui->stackedWidget->setCurrentWidget(ui->page_Microbit);
        } else if (action == ui->actionDfuDownload) {
            ui->stackedWidget->setCurrentWidget(ui->page_DfuDownload);
        } else if (action == ui->actionMCUispDownload) {
            ui->stackedWidget->setCurrentWidget(ui->page_McuispDownload);
        } else if (action == ui->actionMCUispDownload) {
            qDebug() << "action == ui->actionMCUispDownload";
        } else if (action == ui->actionDownloadTest) {
            ui->stackedWidget->setCurrentWidget(ui->page_DownloadTest);
        } else if (action == ui->actionCamera) {
            if (m_monitorWidget == nullptr) {
                m_monitorWidget = new DLocalMonitorForm();
            }
            m_monitorWidget->start();
            m_monitorWidget->show();
        } else if (action == ui->actionRemoteMonitor) {
            if (m_remoteMonitorForm == nullptr) {
                m_remoteMonitorForm = new DRemoteMonitorForm();
            }
            m_remoteMonitorForm->show();
        }
    } else if (menu == ui->menuPlugins) {
        /* PLUGINS */

        QString pluginName;
        if (action == ui->actionMagicDevicePlugin) {
            pluginName = "MagicDevicePlugin";
        } else if (action == ui->actionIndustrialRobotPlugin) {
            pluginName = "IndustrialRobotPlugin";
        } else if (action == ui->actionArduinoPlugin) {
            pluginName = "ArduinoPlugin";
        } else if (action == ui->actionMicrobitPlugin) {
            pluginName = "MicrobitPlugin";
        } else if (action == ui->actionDownloadPlugin) {
            pluginName = "DownloadPlugin";
        } else if (action == ui->actionPluginUpgrade) {
            _startMaintenanceProgress();
        }

        if (!pluginName.isEmpty()) {
            if (action->isChecked()) {
                DMessageCenter::getInstance()->loadPlugin(pluginName);
            } else {
                QMessageBox msgBox;
                msgBox.setText(tr("Unload the plugin?"));
                if (pluginName == "MagicDevicePlugin") {
                    msgBox.setInformativeText(tr("if you unload this plugin, "
                                                 "you will not control Magician,"
                                                 "MagicianLite,MagicBox,M1 etc."));
                } else {
                    msgBox.setInformativeText(tr("choose ok to unload. choose cancel to return."));
                }

                msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Ok) {
                    DMessageCenter::getInstance()->unloadPlugin(pluginName);
                } else {
                    action->setChecked(true);
                }
            }
        }
    }
}

/* systemtray click slot */
void DobotLinkMain::systemTrayActivated_slot(QSystemTrayIcon::ActivationReason reason)
{
#ifdef Q_OS_WIN
    if (reason == QSystemTrayIcon::DoubleClick) {
        this->show();

        qDebug() << "Double clicked Tray.";
//        mSystemTray->contextMenu()->show();
    }
#else
    Q_UNUSED(reason)
    this->show();
#endif
}

/* handle message slot */
void DobotLinkMain::handleShowLogMsg_slot(QString message)
{
    if (!ui->checkBox_print->isChecked()) {
        return;
    }

    /* filter unimportant message */
    if (ui->checkBox_filterGetPose->isChecked() and message.contains("GetPose")) {
        return;
    } else if (ui->checkBox_filterJointAngle->isChecked() and message.contains("jointAngle")) {
        return;
    }

    if (ui->checkBox_userfilter->isChecked()) {
        QString userFilter = ui->lineEdit_userfilter->text();
        if (!userFilter.isEmpty() and message.contains(userFilter)) {
            return;
        }
    }

    ui->plainTextEdit_Monitor->appendPlainText(message);
    QScrollBar *scrollBar = ui->plainTextEdit_Monitor->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void DobotLinkMain::on_btn_clearbox_clicked()
{
    ui->plainTextEdit_Monitor->clear();
}

void DobotLinkMain::on_checkBox_print_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        DSettings::getInstance()->setIsPrintable(true);
    } else if (arg1 == Qt::Unchecked) {
        DSettings::getInstance()->setIsPrintable(false);
    }
}
