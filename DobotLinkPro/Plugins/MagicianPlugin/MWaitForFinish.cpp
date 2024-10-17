#include "MWaitForFinish.h"

#include <QDebug>
#include "DobotDll.h"

MWaitForFinish::MWaitForFinish(int devid, quint64 targetID, MagicianPacket packet, QObject *parent) : QObject(parent),
    m_packet(packet),
    m_devid(devid),
    m_targetid(targetID)
{
    minTimer = new QTimer(this);
    minTimer->setSingleShot(true);
    connect(minTimer, &QTimer::timeout, this, &MWaitForFinish::onMinTimeOut_slot);

    maxTimer = new QTimer(this);
    maxTimer->setSingleShot(true);
    connect(maxTimer, &QTimer::timeout, this, &MWaitForFinish::onMaxTimeOut_slot);

    checkTimer = new QTimer(this);
    checkTimer->setInterval(200);
    connect(checkTimer, &QTimer::timeout, this, &MWaitForFinish::onCheckTimeOut_slot);
}

MWaitForFinish::~MWaitForFinish()
{
    qDebug() << "MWaitForFinish:destroyed";
}

void MWaitForFinish::startWaiting(int minMs, int maxMs)
{
    if (maxMs < minMs) {
        qDebug() << "startWaiting params error";
    }
    minTimer->setInterval(minMs);
    maxTimer->setInterval(maxMs);

    minTimer->start();
    qDebug() << QString("start waiting for finish. [%1,%2]").arg(minMs).arg(maxMs);
}

void MWaitForFinish::stopWaiting()
{
    minTimer->stop();
    maxTimer->stop();
    checkTimer->stop();
    qDebug() << "MWaitForFinish:stopWaiting";
}

/* SLOT */

void MWaitForFinish::onMinTimeOut_slot()
{
    checkTimer->start();
    maxTimer->start();
}

void MWaitForFinish::onMaxTimeOut_slot()
{
    checkTimer->stop();
    qDebug() << "MWaitForFinish: time out! id:" << m_devid;
    emit finish_signal(DobotCommunicate_Timeout);
}

void MWaitForFinish::onCheckTimeOut_slot()
{
    quint64 curIndex = 0;
    int res = GetQueuedCmdCurrentIndex(m_devid, &curIndex);
    if (res != DobotCommunicate_NoError) {
        stopWaiting();
        emit finish_signal(res);
        qDebug() << "MWaitForFinish::GetQueuedCmdCurrentIndex Error";
    } else {
        if (m_targetid == curIndex) {
            stopWaiting();
            emit finish_signal(res);
        }
    }
}
