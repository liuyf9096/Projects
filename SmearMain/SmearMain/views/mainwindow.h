#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void menuTriggered_slot(QAction *action);

    void on_pushButton_process_clicked();
    void on_pushButton_sensor_clicked();
    void on_pushButton_debug_clicked();

    void close_slot();
    void on_pushButton_stainpool_clicked();
};
#endif // MAINWINDOW_H
