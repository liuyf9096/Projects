#ifndef MYTASK_H
#define MYTASK_H

#include <QObject>

class FWebSocketServer;
class MyTask : public QObject
{
    Q_OBJECT
public:
    explicit MyTask(QObject *parent = nullptr);

private:
    FWebSocketServer *mServer;

private slots:
    void onReceiveMessage(const QString &client_id, const QString &message);

};

#endif // MYTASK_H
