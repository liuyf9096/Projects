#ifndef POOL_FORM_H
#define POOL_FORM_H

#include <QWidget>

namespace Ui {
class PoolForm;
}

class QTimer;
class PoolForm : public QWidget
{
    Q_OBJECT

public:
    explicit PoolForm(QWidget *parent = nullptr);
    ~PoolForm();

    void setTitle(const QString &title);
    void setSampleId(const QString &sid);
    void start(int interval);
    void clear();

private slots:
    void onTimer_slot();

private:
    Ui::PoolForm *ui;

    int m_interval;
    QTimer *m_timer;
};

#endif // POOL_FORM_H
