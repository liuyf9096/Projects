#include "pool_form.h"
#include "ui_pool_form.h"

#include <QTimer>

PoolForm::PoolForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PoolForm)
{
    ui->setupUi(this);
    ui->lcdNumber->display("");

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &PoolForm::onTimer_slot);
}

PoolForm::~PoolForm()
{
    delete ui;
}

void PoolForm::setTitle(const QString &title)
{
    ui->label->setText(title);
}

void PoolForm::setSampleId(const QString &sid)
{
    ui->lineEdit->setText(sid);
}

void PoolForm::start(int interval)
{
    if (interval > 999) {
        m_interval = interval * 0.001;
        m_timer->start();
        ui->lcdNumber->display(m_interval);
    } else {
        ui->lcdNumber->display(0);
    }
}

void PoolForm::clear()
{
    ui->lineEdit->setText("");
    ui->lcdNumber->display("");
    m_timer->stop();
}

void PoolForm::onTimer_slot()
{
    m_interval--;
    ui->lcdNumber->display(m_interval);
    if (m_interval <= 0) {
        m_timer->stop();
    }
}
