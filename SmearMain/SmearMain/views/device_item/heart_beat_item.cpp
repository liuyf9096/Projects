#include "heart_beat_item.h"
#include "ui_heart_beat_item.h"

HeartBeatItem::HeartBeatItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HeartBeatItem)
{
    ui->setupUi(this);
    ui->label->hide();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout, this, &HeartBeatItem::timeout_slot);
}

HeartBeatItem::~HeartBeatItem()
{
    delete ui;
}

void HeartBeatItem::setPeriodTime(int interval)
{
    m_timer->setInterval(interval);
}

void HeartBeatItem::showOnce()
{
    m_timer->start();
    ui->label->show();
}

void HeartBeatItem::timeout_slot()
{
    ui->label->hide();
}
