#include "module_base.h"
#include "sql/f_sql_database_manager.h"
#include <QDateTime>

DModuleBase::DModuleBase(const QString &mid, const QString &userid, QObject *parent)
    : QObject(parent)
    , mModuleId(mid)
    , mUserId(userid)
{
    this->setProperty("mid", mid);
    this->setProperty("userid", userid);

    mLogDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    mRecordDb = FSqlDatabaseManager::GetInstance()->getDatebase("record");

    Q_ASSERT(mLogDb);
    Q_ASSERT(mRecordDb);

    qDebug().noquote() << QString("+ %1[%2] created.").arg(mid, userid);
}

void DModuleBase::sendUIMessage(JPacket &p)
{
    FMessageCenter::GetInstance()->sendUIMessage(p);
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
        mLogDb->insertRecord(mLabel, obj);
    }
}

void DModuleBase::recordModuleSampleState(const QString &module, const QString &table, QSharedPointer<RtSample> sample)
{
    if (mRecordDb) {
        QJsonObject setObj;
        if (sample != nullptr) {
            setObj.insert("sid", sample->sid());
            setObj.insert("sample_uid", sample->sampleUID());
        } else {
            setObj.insert("sid", "");
            setObj.insert("sample_uid", "");
        }
        mRecordDb->updateRecord(table, setObj, {{"module", module}});
    }
}

void DModuleBase::recordModuleSampleState(const QString &module, bool isCapped)
{
    if (mRecordDb) {
        QJsonObject setObj;
        if (isCapped == true) {
            setObj.insert("isCapped", 1);
        } else {
            setObj.insert("isCapped", 0);
        }
        mRecordDb->updateRecord("doing_sample", setObj, {{"module", module}});
    }
}

void DModuleBase::recordModuleSlideState(const QString &module, const QString &table, QSharedPointer<RtSlide> slide)
{
    if (mRecordDb) {
        QJsonObject setObj;
        if (slide != nullptr) {
            setObj.insert("slide_id", slide->slide_id());
            setObj.insert("slide_uid", slide->slideUID());
        } else {
            setObj.insert("slide_id", "");
            setObj.insert("slide_uid", "");
        }
        mRecordDb->updateRecord(table, setObj, {{"module", module}});
    }
}

void DModuleBase::recordSlotSlideState(int pos, QSharedPointer<RtSlide> slide)
{
    if (mRecordDb) {
        QJsonObject setObj;
        if (slide != nullptr) {
            setObj.insert("slide_id", slide->slide_id());
            setObj.insert("slide_uid", slide->slideUID());
            setObj.insert("sample_uid", slide->sampleUID());
            setObj.insert("sample_id", slide->sampleID());
        } else {
            setObj.insert("slide_id", "");
            setObj.insert("slide_uid", "");
            setObj.insert("sample_uid", "");
            setObj.insert("sample_id", "");
        }
        mRecordDb->updateRecord("slot", setObj, {{"slot_pos", pos}});
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

