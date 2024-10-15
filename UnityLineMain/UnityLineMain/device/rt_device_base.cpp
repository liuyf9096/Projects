#include "rt_device_base.h"

#include "messagecenter/f_message_center.h"
#include "sql/f_sql_database_manager.h"
#include <QTimer>
#include <QByteArray>
#include <QJsonValue>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>

static const int TIMER_SENDING = 10;

RtDeviceBase::RtDeviceBase(const QString &devid, QObject *parent)
    : QObject(parent)
    , mDeviceId(devid)
{
    m_isEnable = false;
    m_isCheckingSensor = false;
    m_isResetting = false;
    m_isResetError = false;
    m_Lock_1 = false;

    mSendMsgTimer = new QTimer(this);
    mSendMsgTimer->start(TIMER_SENDING);
    connect(mSendMsgTimer, &QTimer::timeout,
            this, &RtDeviceBase::SendPacket_slot);

    /* websocket */
    mMsgCenter = FMessageCenter::GetInstance();
    connect(mMsgCenter, &FMessageCenter::onReceiveCanbusPacket_signal,
            this, &RtDeviceBase::handleCanbusMessage_slot);
    connect(mMsgCenter, &FMessageCenter::onReceiveCanbusNotification_signal,
            this, &RtDeviceBase::handleCanbusNotification_slot);

    /* function init */
    setFunctionMap("Reset", &RtDeviceBase::cmd_Reset);
    setFunctionMap("CheckSensorValue", &RtDeviceBase::cmd_CheckSensorValue);
}

RtDeviceBase::~RtDeviceBase()
{
    qDebug() << mDeviceId << __FUNCTION__;
}

void RtDeviceBase::SetSensorPortMap(const QJsonArray &arr)
{
    mSensorArr = arr;
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString name = obj.value("name").toString();
        int port = obj.value("port").toInt();
        SensorPortMap.insert(name, port);
    }

    if (!SensorPortMap.isEmpty()) {
        qDebug().noquote() << QString("[%1](%2) Set sensors:").arg(mDeviceId, mUsername) << SensorPortMap;
    }
}

void RtDeviceBase::SetFuncTimeoutMap(const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString funcName = obj.value("function").toString();
        int timeout = obj.value("timeout").toInt(0);
        if (timeout > 0) {
            FuncTimeoutMap.insert(funcName, timeout);
        }
        int func_id = obj.value("func_id").toInt();
        if (func_id > 0) {
            FuntionIdMap.insert(funcName, func_id);
        }
    }
    if (!FuncTimeoutMap.isEmpty()) {
        qDebug().noquote() << QString("[%1](%2) Set function timeout:").arg(mDeviceId, mUsername) << FuncTimeoutMap;
    }
}

void RtDeviceBase::reset()
{
    m_sendingList.clear();
    m_requestMap.clear();

    clearTimeoutTimer();
    cmd_Reset();

    qDebug() << mDeviceId << __FUNCTION__;
}

void RtDeviceBase::setCheckSensorEnable(const QString &usr, bool en)
{
    m_sensorReqMap.insert(usr, en);
    int en_count = 0;
    for (const auto& ok : qAsConst(m_sensorReqMap)) {
        if (ok == true) {
            en_count++;
        }
    }

    if (en_count > 0) {
        if (m_isCheckingSensor == false) {
            m_isCheckingSensor = true;
            QJsonObject obj = {{"enable", true}};
            SendCommand("SensorCheckEnable", obj);
        }
    } else {
        if (m_isCheckingSensor == true) {
            m_isCheckingSensor = false;
            QJsonObject obj = {{"enable", false}};
            SendCommand("SensorCheckEnable", obj);
        }
    }
}

bool RtDeviceBase::checkSensorValue(const QString &sensor)
{
    if (SensorPortMap.contains(sensor)) {
        int port = SensorPortMap.value(sensor);
        QString port_s = QString::number(port);
        if (m_sensorObj.contains(port_s)) {
            return m_sensorObj.value(port_s).toBool();
        }
    }
    qDebug() << "Can NOT find sensor:" << sensor;
    return false;
}

bool RtDeviceBase::isResetOk()
{
    if (m_isResetError == true) {
        qDebug() << deviceID() << "is Reset Error, Wait For Reset.";
        return false;
    }
    if (m_isResetting == true) {
        return false;
    }
    return true;
}

void RtDeviceBase::clearTimeoutTimer()
{
    foreach (auto timer, m_funcTimeoutMap) {
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }
    m_funcTimeoutMap.clear();
}

void RtDeviceBase::SendComboActionStart(const QString &api, const QJsonValue &argValue, int timeout)
{
    if (m_isEnable == false) {
        qDebug() << "Device:" << mDeviceId << "is Not Available.";
        return;
    }

    if (api.contains("_Blink") || api.contains("_Breathe")) {
        setFunctionDone(api, Func_Ignore);
    } else {
        setFunctionDone(api, Func_Undone);
    }

    JPacket p(PacketType::Request);
    p.module = mDeviceId;
    p.api = QStringLiteral("ComboActionStart");
    p.subapi = api;

    QJsonObject obj;
    obj.insert("api", api);
    if (argValue.isArray()) {
        obj.insert("arg", argValue);
    }
    obj.insert("timeout", timeout);
    p.paramsValue = obj;

    m_sendingList.append(p);

    if (!mSendMsgTimer->isActive()) {
        mSendMsgTimer->start();
    }
}

void RtDeviceBase::SendComboActionStop(const QString &api)
{
    setFunctionDone(api, Func_Undone);

    JPacket p(PacketType::Request);
    p.module = mDeviceId;
    p.api = "ComboActionStop";
    p.subapi = QString("stop_%1").arg(api);

    QJsonObject obj;
    obj.insert("api", api);
    p.paramsValue = obj;

    m_sendingList.append(p);

    if (!mSendMsgTimer->isActive()) {
        mSendMsgTimer->start();
    }
}

void RtDeviceBase::SendCommand(const QString &command, const QJsonValue &argValue)
{
    if (m_isEnable == false) {
        qDebug() << "Device:" << mDeviceId << "is Not Available.";
        return;
    }

    setFunctionDone(command, Func_Undone);

    JPacket p(PacketType::Request);
    p.module = mDeviceId;
    p.api = command;
    p.paramsValue = argValue;

    m_sendingList.append(p);

    if (!mSendMsgTimer->isActive()) {
        mSendMsgTimer->start();
    }
}

/* delete timers */
void RtDeviceBase::close_slot()
{
    mSendMsgTimer->stop();
    delete mSendMsgTimer;

    auto timerlist = m_funcTimeoutMap.values();
    for(auto timer : qAsConst(timerlist)) {
        if (timer) {
            timer->stop();
            delete timer;
        }
    }
    m_funcTimeoutMap.clear();
}

/* Command */
bool RtDeviceBase::cmd_Reset()
{
    if (m_isResetting) {
        return true;
    }
	
	if (m_Lock_1 == false) {
        m_isResetting = true;
        m_isResetError = false;
        SendComboActionStart("Reset");
        return true;
    }
    return false;
}

bool RtDeviceBase::cmd_SystemTimeSync(int year, int month, int day, int hh, int mm, int sec, int msec)
{
    QJsonObject obj;
    obj.insert("year", year);
    obj.insert("month", month);
    obj.insert("day", day);
    obj.insert("hh", hh);
    obj.insert("mm", mm);
    obj.insert("sec", sec);
    obj.insert("msec", msec);

    SendCommand("SystemTimeSync", obj);
    return true;
}

bool RtDeviceBase::cmd_SystemTimeSync()
{
    QDateTime dt = QDateTime::currentDateTime();
    return cmd_SystemTimeSync(dt.date().year(), dt.date().month(), dt.date().day(),
                              dt.time().hour(), dt.time().minute(), dt.time().second(), dt.time().msec());
}

bool RtDeviceBase::cmd_CheckSensorValue()
{
    SendCommand("CheckSensorValue");
    return true;
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunction fn)
{
    FunctionMap.insert(api, fn);
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunctionArg fn)
{
    FunctionArgMap.insert(api, fn);
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunctionArg1 fn)
{
    FunctionArg1Map.insert(api, fn);
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunctionArg2 fn)
{
    FunctionArg2Map.insert(api, fn);
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunctionArg3 fn)
{
    FunctionArg3Map.insert(api, fn);
}

void RtDeviceBase::setFunctionMap(const QString &api, pFunctionArg4 fn)
{
    FunctionArg4Map.insert(api, fn);
}

bool RtDeviceBase::isFuncDone(const QString &api)
{
    auto resultType = getFuncResult(api);
    if (resultType == Func_Done) {
        return true;
    }
    return false;
}

FuncResultType RtDeviceBase::getFuncResult(const QString &api)
{
    if (m_funcResultMap.contains(api)) {
        return m_funcResultMap.value(api);
    } else {
        qWarning() << deviceID() << "can NOT check Function Done. api:" << api;
    }
    return Func_Undone;
}

void RtDeviceBase::setFunctionDone(const QString &api, FuncResultType result)
{
    if (result != Func_Ignore) {
        m_funcResultMap.insert(api, result);
    } else {
        qDebug() << "setFunctionDone ignore, api:" << api;
    }
}

bool RtDeviceBase::functionContains(const QString &api)
{
    return m_funcResultMap.contains(api);
}

void RtDeviceBase::exeFunction_slot(const QString &api, const QJsonValue &arg)
{
    if (FunctionMap.contains(api)) {
        auto fn = FunctionMap.value(api);
        (this->*fn)();
    } else if (FunctionArg1Map.contains(api)) {
        QJsonArray arr = arg.toArray();
        int arg1 = arr.first().toInt();
        auto fn1 = FunctionArg1Map.value(api);
        (this->*fn1)(arg1);
    } else if (FunctionArg2Map.contains(api)) {
        QJsonArray arr = arg.toArray();
        int arg1 = arr.at(0).toInt();
        int arg2 = arr.at(1).toInt();
        auto fn2 = FunctionArg2Map.value(api);
        (this->*fn2)(arg1, arg2);
    } else if (FunctionArg3Map.contains(api)) {
        QJsonArray arr = arg.toArray();
        int arg1 = arr.at(0).toInt();
        int arg2 = arr.at(1).toInt();
        int arg3 = arr.at(2).toInt();
        auto fn3 = FunctionArg3Map.value(api);
        (this->*fn3)(arg1, arg2, arg3);
    } else if (FunctionArg4Map.contains(api)) {
        QJsonArray arr = arg.toArray();
        int arg1 = arr.at(0).toInt();
        int arg2 = arr.at(1).toInt();
        int arg3 = arr.at(2).toInt();
        int arg4 = arr.at(3).toInt();
        auto fn4 = FunctionArg4Map.value(api);
        (this->*fn4)(arg1, arg2, arg3, arg4);
    } else if (FunctionArgMap.contains(api)) {
        auto fn = FunctionArgMap.value(api);
        (this->*fn)(arg);
    } else {
        qWarning() << "can NOT handle function. api:" << api;
    }
}

void RtDeviceBase::handleCanbusMessage_slot(const JPacket &result, const JPacket &request)
{
    Q_UNUSED(request)

    if (m_requestMap.contains(result.id))
    {
#if 0
        if (result.type == PacketType::Result) {
            qDebug().noquote() << QStringLiteral("[Canbus] >>") << result;
        } else if (result.type == PacketType::Error) {
            qWarning().noquote() << QStringLiteral("[Canbus] >>") << result;
        }
#endif

        QString api = m_requestMap.take(result.id);

        bool ok = handleSelfWSMessage(api, result);
        if (ok == false) {
            if (result.type == PacketType::Result) {
                handleReceiveResult(api, result.resValue);
            } else if (result.type == PacketType::Error) {
                //ERROR[1]
                QJsonObject obj = result.errorObj;
                obj.insert("board_id", mAddress);
                obj.insert("device_id", mDeviceId);
                obj.insert("api", api);

                QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                obj.insert("datetime", datetime);

                if (FuntionIdMap.contains(api)) {
                    int function_id = FuntionIdMap.value(api);
                    obj.insert("function_id", function_id);

                    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
                    QJsonArray arr = db->selectRecord("canbus_exception", QJsonObject({{"function_id", function_id}}));
                    if (arr.count() > 0) {
                        QJsonObject eObj = arr.first().toObject();
                        QString error_id = eObj.value("error_code").toString();
                        QString message = eObj.value("title").toString();
                        QString solution = eObj.value("description").toString();
                        obj.insert("error_id", error_id);
                        obj.insert("message", message);
                        obj.insert("solution", solution);
                        obj.insert("level", 1);
                    }
                }
                handleReceiveResultError(api, obj);

                emit sendInfo_signal(mDeviceId, 80, QString("id:%1 api:%2 function exe fails.")
                                     .arg(result.id).arg(api));
            }
        }

        if (m_funcTimeoutMap.contains(result.id)) {
            auto timer = m_funcTimeoutMap.take(result.id);
            timer->stop();
            delete timer;
        }
    }
}

void RtDeviceBase::handleCanbusNotification_slot(const JPacket &notification)
{
    if (notification.type == PacketType::Notification) {
        if (notification.device == QStringLiteral("CanbusServer")) {
            if (notification.module == mDeviceId) {
                handleNoticification(notification);
            }
        }
    }
}

bool RtDeviceBase::handleSelfWSMessage(const QString &api, const JPacket &result)
{
    Q_UNUSED(result)
    if (api == "SensorCheckEnable") {
        return true;
    }
    return false;
}

bool RtDeviceBase::handleReceiveResult(const QString &api, const QJsonValue &resValue)
{
    if (api == "CheckSensorValue") {
        m_sensorObj = resValue.toObject();
    }

    if (functionContains(api)) {
        setFunctionDone(api, Func_Done);

        if (api == "Reset") {
            m_isResetting = false;
            m_isResetError = false;
        }
    } else {
        qWarning() << QString("%1 can NOT handle receive result. api:%3").arg(deviceID(), api) << resValue;
    }

    emit onFunctionFinished_signal(api, resValue);
    return true;
}

void RtDeviceBase::handleReceiveResultError(const QString &api, const QJsonObject &errorObj)
{
    if (api.endsWith("_Blink")) {
        return;
    }

    //ERROR[2]
    int code = errorObj.value("code").toInt();
    QString message = errorObj.value("message").toString();

    if (functionContains(api)) {
        setFunctionDone(api, Func_Fail);

        if (api == "Reset") {
            m_isResetting = false;
            m_isResetError = true;
            qWarning() << mDeviceId << "Reset Error.";
            emit sendInfo_signal(mDeviceId, 1, "reset error");
        }
    } else {
        qWarning() << QString("%1 can NOT handle receive error. api:%3").arg(deviceID(), api);
    }

    qWarning() << QString("%1 command error. api:%3 code:%4 message:%5")
                  .arg(deviceID(), api).arg(code).arg(message);

    emit onFunctionFailed_signal(api, errorObj);

#if 0
    JPacket p(PacketType::Notification);
    p.module = mDeviceId;
    p.api = "Alarm";

    QJsonObject obj;
    obj.insert("code", code);
    obj.insert("message", message);
    obj.insert("api", api);
    p.paramsValue = obj;

    mMsgCenter->sendUIMessage(p);
#endif
}

void RtDeviceBase::handleNoticification(const JPacket &p)
{
    if (p.module == mDeviceId) {
        if (p.api == "SensorValue") {
            m_sensorObj = p.paramsValue.toObject();
            emit sonserInfo_signal(mDeviceId, m_sensorObj);
        } else {
            qDebug() << mDeviceId << "Unknown Notification:" << p;
        }
    }
}

/* send command slot */
void RtDeviceBase::SendPacket_slot()
{
    if (m_sendingList.isEmpty()) {
        mSendMsgTimer->stop();
    } else if (mMsgCenter->isCanbusConnected()) {
        JPacket packet = m_sendingList.takeFirst();

        mMsgCenter->sendCanbusMessage(packet, mDeviceId); // assign p.id value

        QString api = packet.subapi.isEmpty() ? packet.api : packet.subapi;
        m_requestMap.insert(packet.id, api);

        /* timeout */
        if (FuncTimeoutMap.contains(api))
        {
            int timeout = FuncTimeoutMap.value(api);
            if (timeout > 0) {
                QTimer *timer = new QTimer(this);
                timer->setProperty("id", packet.id);
                timer->setProperty("api", api);
                timer->setSingleShot(true);
                connect(timer, &QTimer::timeout, this, &RtDeviceBase::handleTimeoutPacket_slot);

                m_funcTimeoutMap.insert(packet.id, timer);
                timer->start(timeout);
            }
        }
    }
}

/* Timeout */
void RtDeviceBase::handleTimeoutPacket_slot()
{
    auto timer = qobject_cast<QTimer*>(sender());
    if (timer) {
        QString api = timer->property("api").toString();
        int id = timer->property("id").toInt();
        m_funcTimeoutMap.remove(id);
        m_requestMap.remove(id);
        setFunctionDone(api, Func_Timeout);
        timer->deleteLater();

        emit onFunctionTimeout_signal(api);
        emit sendInfo_signal(mDeviceId, 99, QString("[%1] id:%2 timeout.").arg(mDeviceId).arg(id));
    }
}
