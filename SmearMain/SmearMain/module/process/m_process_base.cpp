#include "m_process_base.h"
#include "messagecenter/f_message_center.h"
#include "sql/f_sql_database_manager.h"

MProcessBase::MProcessBase(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase{mid, userid, parent}
{
    _init();

    mRecordDb = FSqlDatabaseManager::GetInstance()->getDatebase("record");

    mTimer = new QTimer(this);
    mTimer->setInterval(100);
    connect(mTimer, &QTimer::timeout, this, &MProcessBase::onTimer_slot);
}

void MProcessBase::_init()
{
    m_isFinished = true;
    m_isError = false;
    m_reqId = 0;
}

bool MProcessBase::startProcess()
{
    if (m_isFinished == true) {
        _init();
        state_init();
        m_isFinished = false;

        if (mTimer->isActive() == false) {
            mTimer->start();
        }
        qInfo() << "Start Process:" << mUserId;
        return true;
    }
    return false;
}

bool MProcessBase::startProcess(quint64 id)
{
    if (m_isFinished == true) {
        _init();
        state_init();
        m_isFinished = false;

        m_reqId = id;
        if (mTimer->isActive() == false) {
            mTimer->start();
        }
        qInfo() << "Start Process:" << mUserId << "id:" << m_reqId;
        return true;
    }
    return false;
}

void MProcessBase::resumeProcess()
{
    mTimer->start();
}

void MProcessBase::stopProcess(quint64 id)
{
    mTimer->stop();
    sendOkMessage(id);
}

void MProcessBase::stopProcess()
{
    mTimer->stop();
}

void MProcessBase::resetProcess(quint64 id)
{
    _init();
    state_init();
    m_reqId = id;
}

void MProcessBase::resetProcess()
{
    _init();
    state_init();
}

void MProcessBase::sendFinishSignal()
{
    emit onProcessFinished_signal(mUserId);
}

void MProcessBase::sendErrorSignal()
{
    emit onProcessError_signal(mUserId);
}

bool MProcessBase::checkStateAllFinished()
{
    foreach (bool ok, m_ProcessMap) {
        if (ok == false) {
            return false;
        }
    }
    return true;
}

void MProcessBase::sendOkMessage(quint64 id)
{
    JPacket p(PacketType::Result, id);
    p.resValue = true;
    sendUIMessage(p);
}

void MProcessBase::sendOkMessage()
{
    JPacket p(PacketType::Result, m_reqId);
    p.resValue = true;
    sendUIMessage(p);
}

void MProcessBase::sendErrorMessage(quint64 id, const QString &msg)
{
    JPacket p(PacketType::Error, id);
    p.errorCode = 90;
    p.errorMessage = QString("process Error. %1").arg(msg);
    sendUIMessage(p);
}

void MProcessBase::sendErrorMessage(const QString &msg)
{
    JPacket p(PacketType::Error, m_reqId);
    p.errorCode = 90;
    p.errorMessage = QString("process Error. %1").arg(msg);
    sendUIMessage(p);
}

void MProcessBase::sendProcessMessage(int percent, const QString &process, const QString &message)
{
    JPacket p(PacketType::Notification);
    p.module = process;
    p.api = "Progress";

    QJsonObject obj;
    obj.insert("percent", percent);
    obj.insert("message", message);
    p.paramsValue = obj;

    sendUIMessage(p);
}
