#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QJsonObject>

#include "PluginItemForm.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DNetworkManager;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void checkNewVersion();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::MainWindow *ui;

    QStringList m_pluginList;
    DNetworkManager *m_manager;
    QSettings *m_settings;
    QString m_url;
    QMap<QString, QJsonObject> m_newVersionInfo;
    QMap<QString, PluginItemForm*> m_pluginForm;
    QMap<QString, QString> m_pluginFiles;
    QStringList m_downloadList;

    void settingsInit();
    void listInit();
    void setLocalVersion();
    void updateWidget();

    void handlePluginObj(QJsonObject obj);

    inline PluginItemForm *getItemForm(int row);
    int compareVersion(QString v1, QString v2);

private slots:
    void onMenuTriggered_slot(QAction *action);
    void startDownload_slot(QString pluginName);
    void cancelDownload_slot(QString pluginName);

    void onReplyMassage_slot(QString url, QJsonObject obj);
    void downloadFinished_slot(QString url, QString fileName);
    void handleProgress_slot(qint64 bytesReceived, qint64 bytesTotal, QString fileName);
};

#endif // MAINWINDOW_H
