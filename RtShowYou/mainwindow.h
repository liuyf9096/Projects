#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ChangeDecripionDialog;
class QUdpSocket;
class QStandardItemModel;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_send_clicked();
    void on_pushButton_clicked();

    void onReadDatagram_slot();
    void sendMessage_slot(const QString &message);

private:
    Ui::MainWindow *ui;
    QUdpSocket *m_udp;

    quint16 m_bindPort;
    quint16 m_sendPort;

    QStandardItemModel *m_model;
    ChangeDecripionDialog *m_dialog;

    void addMessage(const QString &message);
    void updateForm(const QString &message);

    bool sendUdpMessage(const QString &message);
};

class RCenterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RCenterDelegate(QObject *parent = nullptr);
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // MAINWINDOW_H
