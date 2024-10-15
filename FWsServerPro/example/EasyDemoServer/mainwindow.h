#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FWebSocketServer;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void onTimer_slot();

    void on_pushButton_open_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::MainWindow *ui;

    FWebSocketServer *m_wsServer;
    QString m_clientId;
    QTimer *m_timer;
};
#endif // MAINWINDOW_H
