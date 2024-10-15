#ifndef TASKHANDLER_H
#define TASKHANDLER_H

#include <QObject>

#include "messagecenter/f_message_center.h"

class TaskHandler : public QObject
{
    Q_OBJECT
public:
    static TaskHandler *GetInstance();

private:
    explicit TaskHandler(QObject *parent = nullptr);

private slots:
    void handleReceivePacket_slot(const QString &dev_id, const JPacket &p);
};

#endif // TASKHANDLER_H
