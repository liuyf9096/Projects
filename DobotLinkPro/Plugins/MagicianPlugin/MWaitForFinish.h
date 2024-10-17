#ifndef MWAITFORFINISH_H
#define MWAITFORFINISH_H

#include <QObject>
#include <QTimer>
#include "MagicianPacket.h"

class MWaitForFinish : public QObject
{
    Q_OBJECT
public:
    explicit MWaitForFinish(int devid, quint64 targetID, MagicianPacket packet, QObject *parent = nullptr);
    ~MWaitForFinish();

    double m_cmdID;
    MagicianPacket m_packet;

    void startWaiting(int minMs, int maxMs);
    void stopWaiting();

signals:
    void finish_signal(int res);

private:
    int m_devid;
    quint64 m_targetid;

    QTimer *minTimer;
    QTimer *maxTimer;
    QTimer *checkTimer;

private slots:
    void onMinTimeOut_slot();
    void onMaxTimeOut_slot();
    void onCheckTimeOut_slot();
};

#endif // MWAITFORFINISH_H
