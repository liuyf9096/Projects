#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QThread;
class QTimer;
class FWsClient;
class JPacket;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_sendobj_clicked();
    void on_pushButton_sendtext_clicked();
    void on_pushButton_starttest_clicked();
    void on_pushButton_stoptest_clicked();

    void onReceiveMessagePacket_slot(const JPacket &packet);
    void onReceiveMessage_slot(const QString &message);

private:
    Ui::MainWindow *ui;

    quint64 m_id;

    FWsClient *mClient;
    QTimer *mTimer;
    QThread *m_tread;
};
#endif // MAINWINDOW_H
