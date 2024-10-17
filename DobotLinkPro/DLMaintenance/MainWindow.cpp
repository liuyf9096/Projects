#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QJsonDocument>
#include <QShowEvent>
#include <QListWidgetItem>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

#include "DNetworkManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settingsInit();
    listInit();
    setLocalVersion();

    connect(ui->menuTools, &QMenu::triggered,
            this, &MainWindow::onMenuTriggered_slot);

    m_manager = DNetworkManager::getInstance();
    connect(m_manager, &DNetworkManager::onReplyMassage_signal,
            this, &MainWindow::onReplyMassage_slot);
    connect(m_manager, &DNetworkManager::downloadProgress_signal,
            this, &MainWindow::handleProgress_slot);
    connect(m_manager, &DNetworkManager::downloadFinished_signal,
            this, &MainWindow::downloadFinished_slot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::settingsInit()
{
    QString appPath = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    QDir dir(appPath);

#ifdef Q_OS_WIN
    m_settings = new QSettings(dir.absoluteFilePath("dobotlink.ini"), QSettings::IniFormat, this);
#elif defined (Q_OS_MAC)
    m_settings = new QSettings(dir.absoluteFilePath("dobotlink.plist"), QSettings::NativeFormat, this);
#endif
    m_settings->setIniCodec("UTF-8");
}

void MainWindow::listInit()
{
    m_settings->beginGroup("plugin");
    m_pluginList = m_settings->childKeys();
    m_settings->endGroup();

    ui->listWidget->setMinimumHeight(51 * m_pluginList.count());
    ui->listWidget->setMaximumHeight(51 * m_pluginList.count());

    for (int i = 0; i < m_pluginList.count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(500, 50));
        ui->listWidget->addItem(item);
        PluginItemForm *form = new PluginItemForm(m_pluginList.at(i), ui->listWidget);
        m_pluginForm.insert(m_pluginList.at(i), form);
        connect(form, &PluginItemForm::onStartUpdate_signal, this, &MainWindow::startDownload_slot);
        connect(form, &PluginItemForm::onCancelUpdate_signal, this, &MainWindow::cancelDownload_slot);
        ui->listWidget->setItemWidget(item, form);
    }
}

void MainWindow::setLocalVersion()
{
    m_settings->beginGroup("version");
    for (int i = 0; i < m_pluginList.count(); ++i) {
        PluginItemForm *form = getItemForm(i);
        QString version = m_settings->value(form->pluginName).toString();
        if (version.contains("(")) {
            version = version.left(version.indexOf("("));
        }
        form->setLocalVersion(version);
    }
    m_settings->endGroup();
}

void MainWindow::checkNewVersion()
{
    qDebug() << "start checking new version.";

    m_manager->getDobotRequest("/DobotLink/version.json");
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    checkNewVersion();
}

void MainWindow::onMenuTriggered_slot(QAction *action)
{
    if (action == ui->actionfresh_version) {
        checkNewVersion();
    }
}

PluginItemForm *MainWindow::getItemForm(int row)
{
    if (row < ui->listWidget->count()) {
        QWidget *w = ui->listWidget->itemWidget(ui->listWidget->item(row));
        PluginItemForm *form = qobject_cast<PluginItemForm *>(w);
        if (form) {
            return form;
        }
    }
    return nullptr;
}

void MainWindow::onReplyMassage_slot(QString url, QJsonObject obj)
{
    if (url.endsWith("DobotLink/version.json")) {
        QJsonObject pluginObj = obj.value("plugins").toObject();
        handlePluginObj(pluginObj);
    }
}

void MainWindow::handlePluginObj(QJsonObject obj)
{
//    qDebug() << "get" << obj;

    m_settings->beginGroup("plugin");
    foreach (const QString &pluginName, m_pluginList) {
        if (obj.contains(pluginName)) {
            QJsonObject MagicDeviceObj = obj.value(pluginName).toObject();
            m_newVersionInfo.insert(pluginName, MagicDeviceObj);
        }
    }
    m_settings->endGroup();

    updateWidget();
}

void MainWindow::updateWidget()
{
    m_settings->beginGroup("version");

    for (int i = 0; i < m_pluginList.count(); ++i) {
        PluginItemForm *form = getItemForm(i);
        QJsonObject infoObj = m_newVersionInfo.value(form->pluginName);

        QString oldVersion = m_settings->value(form->pluginName).toString();
        if (oldVersion.contains("(")) {
            oldVersion = oldVersion.left(oldVersion.indexOf("("));
        }
        form->setLocalVersion(oldVersion);

        QString newVersion = infoObj.value("version").toString();
        form->setLatestVersion(newVersion);

        int a = compareVersion(oldVersion, newVersion);
        if (a < 0) {
            form->setInfoState(PluginItemForm::NEW_VERSION);
        } else {
            form->setInfoState(PluginItemForm::NONEW);
        }
    }

    m_settings->endGroup();
}

int MainWindow::compareVersion(QString v1, QString v2)
{
    QStringList v1list = v1.split(".", QString::SkipEmptyParts);
    QStringList v2list = v2.split(".", QString::SkipEmptyParts);

    int len1 = v1list.count();
    int len2 = v2list.count();
    int i;
    for (i = 0; i < qMin(len1, len2); ++i) {
        if (v1list.at(i).toUInt() > v2list.at(i).toUInt()) {
            return 1;
        } else if (v1list.at(i).toUInt() < v2list.at(i).toUInt()) {
            return -1;
        }
    }
    if (v1list.count() > v2list.count()) {
        for (int j = i; j < len1; ++j) {
            if (v1list.at(j).toUInt() != 0) {
                return 1;
            }
        }
    } else if (v1list.count() < v2list.count()) {
        for (int j = i; j < len2; ++j) {
            if (v2list.at(j).toUInt() != 0) {
                return 1;
            }
        }
    }
    return 0;
}

void MainWindow::startDownload_slot(QString pluginName)
{
    QJsonObject infoObj = m_newVersionInfo.value(pluginName);
    qDebug() << "downloading.." << pluginName;

    QJsonObject urlObj = infoObj.value("url").toObject();
#ifdef Q_OS_WIN
    QJsonArray urlArray = urlObj.value("win").toArray();
#elif defined (Q_OS_MAC)
    QJsonArray urlArray = urlObj.value("mac").toArray();
#endif

    QString firstFile;

    for (int i = 0; i < urlArray.count(); ++i) {
        QString url = urlArray.at(i).toString();
        QFileInfo info(url);
        m_pluginFiles.insert(info.fileName(), pluginName);
        if (i == 0) {
            firstFile = info.fileName();
        }

        m_manager->downloadPlugin(url);
        m_downloadList.append(url);

        ui->statusbar->showMessage(url, 500);
    }

    ui->statusbar->showMessage(tr("start downloading %1").arg(firstFile), 500);
}

void MainWindow::cancelDownload_slot(QString pluginName)
{
    qDebug() << "cancel" << pluginName;
    m_manager->cancelDownload();
}

void MainWindow::downloadFinished_slot(QString url, QString fileName)
{
    qDebug() << "download finished." << url << fileName;
    m_downloadList.removeOne(url);

    QString name = fileName.remove("{$$}");
    QFileInfo info(name);    

    if (m_downloadList.isEmpty()) {
        ui->statusbar->showMessage(tr("download finish."), 500);
        QString pluginName = m_pluginFiles.value(info.fileName());
        PluginItemForm *form = m_pluginForm.value(pluginName);
        if (form) {
            form->setInfoState(PluginItemForm::FINISH);
        }
    } else {
        ui->statusbar->showMessage(info.fileName() + " download finish.");
    }
}

void MainWindow::handleProgress_slot(qint64 bytesReceived, qint64 bytesTotal, QString fileName)
{
    QString pluginName = m_pluginFiles.value(fileName);
    PluginItemForm *form = m_pluginForm.value(pluginName);
    if (form) {
        double percent = bytesReceived * 100 / bytesTotal;
        form->setProgressBar(static_cast<int>(percent));
    }
}

