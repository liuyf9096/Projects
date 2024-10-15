#include "exception_center.h"
#include "messagecenter/f_message_center.h"
#include "sql/f_sql_database_manager.h"
#include "device/rt_device_base.h"

#include <QDateTime>
#include <QDebug>

const QString DEV_ID = "United";
static const QStringList ItemTypeList = {"can", "serial", "memory", "function", "motor", "optocoupler"};

Exception::Exception(const QString &error_id, E_Level level, int code)
    : e_id(error_id)
    , e_code(code)
    , e_level(level)
{
    e_type = Exception_Type::Unknown;
    e_module = "Exception";
}

ExceptionCenter *ExceptionCenter::GetInstance()
{
    static ExceptionCenter *instance = nullptr;
    if (instance == nullptr) {
        instance = new ExceptionCenter();
    }
    return instance;
}

QString ExceptionCenter::getItemTypeS(int itemType)
{
    if (itemType > 0 && itemType < ItemTypeList.count()) {
        return ItemTypeList.at(itemType);
    }
    return QString();
}

QString ExceptionCenter::getErrorTypeS(quint8 itemType, quint8 errorType)
{
    if (itemType == 6) {    // optocoupler
        switch (errorType) {
        case 1:
            return "Cover-Error";
            break;
        case 2:
            return "UnCover-Error";
            break;
        case 3:
            return "Unknow-Coverage";
            break;
        default:
            break;
        }
    } else if (itemType == 5) { // motor
        switch (errorType) {
        case 1:
            return "Reset-Error";
            break;
        case 2:
            return "PreReset-Error";
            break;
            /* ... */
        default:
            break;
        }
    }
    return QString();
}

ExceptionCenter::ExceptionCenter(QObject *parent)
    : QObject{parent}
{
    mLogDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    mConfigDb = FSqlDatabaseManager::GetInstance()->getDatebase("config");
}

/* todo */
void ExceptionCenter::addException(const QString &device, const QString &api, Exception_Type type, const QString &usercode)
{
    QString message, solution, type_1;
    int level = 0;
    int code = 0;

    /* 1.generate exception_id */
    QString exception_id = QString("%1").arg(api);
    if (type == Exception_Type::Action_Fail) {
        type_1 = "Action_Fail";
        exception_id.append("_80");
    } else if (type == Exception_Type::Action_Timeout) {
        type_1 = "Action_Timeout";
        exception_id.append("_99");
    } else if (type == Exception_Type::UserCode) {
        type_1 = "UserCode";
        exception_id.append(usercode);
    }

    /* 2.match solution,level,message with exception_id */
    QJsonObject w1Obj;
    w1Obj.insert("device", device);
    w1Obj.insert("exception_id", exception_id);

    QJsonArray sel1 = mConfigDb->selectRecord("exception", w1Obj);
    if (sel1.count() > 0) {
        QJsonObject obj = sel1.first().toObject();
        message = obj.value("message").toString();
        solution = obj.value("solution").toString();
        level =  obj.value("level").toInt();
        code =  obj.value("code").toInt();
    }

    /* 3.insert exception into logfile */
    QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QJsonObject setObj;
    setObj.insert("datetime", datetime);

    QString error_id;
    if (device == "main") {
        error_id = exception_id;
    } else {
        error_id = QString("%1_%2").arg(device, exception_id);
    }

    QJsonObject w2Obj;
    w2Obj.insert("error_id", error_id);
    QJsonArray sel2 = mLogDb->selectRecord("exception", w2Obj);
    if (sel2.count() > 0) {
        mLogDb->updateRecord("exception", setObj, w2Obj);
    } else {
        setObj.insert("error_id", error_id);
        setObj.insert("message", message);
        setObj.insert("solution", solution);
        setObj.insert("level", level);
        setObj.insert("code", code);
        setObj.insert("device", device);
        setObj.insert("api", api);
        setObj.insert("type", type_1);
        mLogDb->insertRecord("exception", setObj);
    }

    /* 4. Send message to UI */
    QJsonObject messageObj;
    messageObj.insert("error_id", error_id);
    messageObj.insert("datetime", datetime);
    messageObj.insert("error_id", error_id);
    messageObj.insert("message", message);
    messageObj.insert("solution", solution);
    messageObj.insert("level", level);
    messageObj.insert("code", code);
    messageObj.insert("device", device);
    messageObj.insert("api", api);
    messageObj.insert("type", type_1);

    sendExceptionMessage(messageObj);

    m_errorMap.insert(error_id, messageObj);
}

void ExceptionCenter::sendExceptionMessage(QJsonObject obj)
{
    obj.insert("dev_id", DEV_ID);

    JPacket p(PacketType::Notification);
    p.module = QStringLiteral("Exception");
    p.api = QStringLiteral("SetException");
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void ExceptionCenter::removeException(const QString &error_id)
{
    QJsonObject whereObj;
    whereObj.insert("error_id", error_id);

    mLogDb->deleteRecord("exception", whereObj);
    sendRemoveExceptionMessage(error_id);
}

void ExceptionCenter::removeException(const QString &device, const QString &api)
{
    QJsonObject whereObj;
    whereObj.insert("device", device);
    whereObj.insert("api", api);

    QStringList list;
    QJsonArray sel = mLogDb->selectRecord("exception", whereObj);
    if (sel.count() > 0) {
        for (int i = 0; i < sel.count(); ++i) {
            QJsonObject obj = sel.at(i).toObject();
            QString error_id = obj.value("error_id").toString();
            list.append(error_id);
            m_errorMap.remove(error_id);
        }
    }
    mLogDb->deleteRecord("exception", whereObj);
    sendRemoveExceptionMessages(list);    
}

void ExceptionCenter::sendRemoveExceptionMessage(const QString &error_id)
{
    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "ClearException";

    QJsonObject obj;
    QJsonArray arr;
    arr.append(error_id);
    obj.insert("error_id", arr);
    obj.insert("dev_id", DEV_ID);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void ExceptionCenter::sendRemoveExceptionMessages(const QStringList &list)
{
    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "ClearException";

    QJsonObject obj;
    QJsonArray arr;
    for (auto error_id : list) {
        arr.append(error_id);
    }
    obj.insert("error_id", arr);
    obj.insert("dev_id", DEV_ID);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

QJsonArray ExceptionCenter::getAllException()
{
    QJsonArray arr = mLogDb->selectRecord("exception");
    return arr;
}

void ExceptionCenter::onFunctionFinished_slot(const QString &api, const QJsonValue &)
{
    Q_UNUSED(api)
}

void ExceptionCenter::onFunctionFailed_slot(const QString &api, const QJsonObject &errorObj)
{
    Q_UNUSED(errorObj)

    auto device = qobject_cast<RtDeviceBase*>(sender());
    Q_ASSERT(device);

    if (api == "Import_Load") {
        return;
    }

//    addException(device->deviceID(), api, Exception_Type::Action_Fail);

    //ERROR[3]
    int function_id = errorObj.value("function_id").toInt();
    QString error_id = QString("%1%2").arg(device->boardNum(), 2, 10, QChar('0')).arg(function_id, 3, 10, QChar('0'));
    QString error_code = QString("%1%2").arg(error_id).arg(0, 2, 10, QChar('0'));

    QJsonObject obj = errorObj;
    obj.insert("source", "device");
    obj.insert("dev_id", DEV_ID);
    obj.insert("code", 80);
    obj.insert("error_code", error_code);

    Exception exp(error_id);
    exp.e_type = Exception_Type::Action_Fail;
    exp.e_level = E_Level::Error;
    exp.e_code = 80;

    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);

    m_ExpMap.insert(error_id, exp);
}

void ExceptionCenter::onFunctionTimeout_slot(const QString &api)
{
    auto device = qobject_cast<RtDeviceBase*>(sender());
    Q_ASSERT(device);

    addException(device->deviceID(), api, Exception_Type::Action_Timeout);
}
