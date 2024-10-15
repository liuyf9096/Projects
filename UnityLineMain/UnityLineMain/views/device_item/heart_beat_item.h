#ifndef HEART_BEAT_ITEM_H
#define HEART_BEAT_ITEM_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class HeartBeatItem;
}

class HeartBeatItem : public QWidget
{
    Q_OBJECT

public:
    explicit HeartBeatItem(QWidget *parent = nullptr);
    ~HeartBeatItem();

    void setPeriodTime(int interval);

public slots:
    void showOnce();

private:
    Ui::HeartBeatItem *ui;
    QTimer *m_timer;

private slots:
    void timeout_slot();
};

#endif // HEART_BEAT_ITEM_H
