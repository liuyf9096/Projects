#ifndef M_CONFIG_TASK_H
#define M_CONFIG_TASK_H

#include "m_process_base.h"

class MConfigTask : public MProcessBase
{
    Q_OBJECT
public:
    explicit MConfigTask(QObject *parent = nullptr);

    void setSystemTime(quint64 id, const QJsonObject &obj);
    void setSystemTime(int year, int month, int day, int hh, int mm, int sec, int msec = 0);

protected:
    virtual void state_init() override;

protected slots:
    virtual void onTimer_slot() override;

private:
    void _init();

    quint64 m_id;
};

#endif // M_CONFIG_TASK_H
