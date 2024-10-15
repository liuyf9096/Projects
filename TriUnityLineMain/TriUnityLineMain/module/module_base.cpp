#include "module_base.h"
#include "sql/f_sql_database_manager.h"

DModuleBase::DModuleBase(const QString &mid, const QString &userid, QObject *parent)
    : QObject(parent)
    , mModuleId(mid)
    , mUserId(userid)
{
    this->setProperty("mid", mid);
    this->setProperty("userid", userid);

    mLogDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    qDebug().noquote() << QString("+ %1[%2] created.").arg(mid, userid);
}

void DModuleBase::sendUIMessage(JPacket &p)
{
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void DModuleBase::sendUIAlarm(AlarmType type, const QString &alarm_id, int code, const QString &message)
{
    JPacket p(PacketType::Notification);
    p.module = "Module";

    QJsonObject obj;
    if (type == Alarm) {
        p.api = "Alarm";
        obj.insert("alarm_id", alarm_id);
    } else if (type == Error) {
        p.api = "Error";
        obj.insert("error_id", alarm_id);
    }
    obj.insert("code", code);
    obj.insert("message", message);

    p.paramsValue = obj;

    sendUIMessage(p);
}

void DModuleBase::logProcess(const QString &process, int state, const QString &statemsg, const QString &message)
{
    QJsonObject obj;
    obj.insert("time", QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz"));

    QString str;
    if (state >= 0) {
        str = QString("{%1}(%2 %3)").arg(process).arg(state).arg(statemsg);
    } else {
        str = QString("{%1} %2").arg(process, statemsg);
    }

    if (message.isEmpty() == false) {
        str.append(" ");
        str.append(message);
    }
    obj.insert(mModuleId, str);

    if (mLogDb) {
        mLogDb->insertRecord("states", obj);
    }
}

void DModuleBase::start()
{
    qDebug() << mModuleId << "start.";
}

void DModuleBase::reset()
{
    qDebug() << mModuleId << "reset.";
}

void DModuleBase::stop()
{
    qDebug() << mModuleId << "stop.";
}

void DModuleBase::showMessage(const QJsonObject &obj)
{
    emit onDisplayMessage_signal(mModuleId, obj);
}

//void DModuleBase::onDevSensorUpdate_slot(const QString &dev_id, const QJsonObject &obj)
//{
//    Q_UNUSED(dev_id)
//    Q_UNUSED(obj)

//    m_sensorUpdate = true;
//}

