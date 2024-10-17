#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../include/MagicDevice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_search_clicked();
    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();
    void on_pushButton_send_clicked();
    void on_pushButton_send2_clicked();
    void on_pushButton_send3_clicked();
    void on_pushButton_send4_clicked();

    void on_pushButton_m1_clicked();

private:
    Ui::MainWindow *ui;

    MagicDevice *device;
    quint64 id;
};

#endif // MAINWINDOW_H
