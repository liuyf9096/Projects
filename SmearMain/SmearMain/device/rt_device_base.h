#ifndef RT_DEVICE_BASE_H
#define RT_DEVICE_BASE_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

enum FuncResultType {
    Func_Undone = 0,
    Func_Done,
    Func_Fail,
    Func_Ignore,
    Func_Timeout
};

class FMessageCenter;
class QTimer;
class JPacket;
class RtDeviceBase : public QObject
{
    friend class RtDeviceManager;

    Q_OBJECT
public:
    explicit RtDeviceBase(const QString &devid, QObject *parent = nullptr);
    virtual ~RtDeviceBase();

public:
    typedef bool (RtDeviceBase::*pFunction)();
    typedef bool (RtDeviceBase::*pFunctionArg1)(int arg1);
    typedef bool (RtDeviceBase::*pFunctionArg2)(int arg1, int arg2);
    typedef bool (RtDeviceBase::*pFunctionArg3)(int arg1, int arg2, int arg3);
    typedef bool (RtDeviceBase::*pFunctionArg4)(int arg1, int arg2, int arg3, int arg4);
    typedef bool (RtDeviceBase::*pFunctionArg5)(int arg1, int arg2, int arg3, int arg4, int arg5);
    typedef bool (RtDeviceBase::*pFunctionArg6)(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
    typedef bool (RtDeviceBase::*pFunctionArg7)(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7);
    typedef bool (RtDeviceBase::*pFunctionArg8)(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8);
    typedef bool (RtDeviceBase::*pFunctionArg9)(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9);
    typedef bool (RtDeviceBase::*pFunctionArg10)(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);
    typedef bool (RtDeviceBase::*pFunctionArgS)(QString arg);
    typedef bool (RtDeviceBase::*pFunctionArgIS)(int arg1, QString arg2);
    typedef bool (RtDeviceBase::*pFunctionArg)(QJsonValue arg);

    void reset();

    virtual void stateInit() {}

    void SetSensorPortMap(const QJsonArray &arr);
    void SetFuncTimeoutMap(const QJsonArray &arr);

    void setAddress(quint16 address) { mAddress = address; }
    quint16 address() { return mAddress; }
    void setUsername(const QString &username) { mUsername = username; }
    QString username() { return mUsername; }
    QString deviceID() { return mDeviceId; }
    void setBoardNum(int num) { mBoardNum = num; }
    int boardNum() { return mBoardNum; }
    void setEnable(bool en) { m_isEnable = en; }
    bool isEnable() { return m_isEnable; }

    /* sensor data */
    void setCheckSensorEnable(const QString &usr, bool en);
    void setCheckSensorEnable(bool en);
    bool getCheckSensorEnable() { return m_isCheckingSensor; }
    bool checkSensorValue(const QString &sensor);
    bool checkSensorValueOr(const QString &s1, const QString &s2);
    bool checkSensorValueOr(const QString &s1, const QString &s2, const QString &s3);
    bool checkSensorValueOr(const QString &s1, const QString &s2, const QString &s3, const QString &s4);
    bool checkSensorValueOr(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5);
    bool checkSensorValueOr(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5, const QString &s6);
    bool checkSensorValueAnd(const QString &s1, const QString &s2);
    bool checkSensorValueAnd(const QString &s1, const QString &s2, const QString &s3);
    bool checkSensorValueAnd(const QString &s1, const QString &s2, const QString &s3, const QString &s4);
    bool checkSensorValueAnd(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5);
    bool checkSensorValueAnd(const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5, const QString &s6);

    int checkFloaterSensorValue(const QString &fsensor);
    bool setCheckFloaterSensorEnable(bool on, int count);

    /* commond */
    QStringList getCommandList() { return FunctionMap.keys(); }
    QStringList getCommandArgList() { return FunctionArgMap.keys(); }
    QStringList getCommandArg1List() { return FunctionArg1Map.keys(); }
    QStringList getCommandArg2List() { return FunctionArg2Map.keys(); }
    QStringList getCommandArg3List() { return FunctionArg3Map.keys(); }
    QStringList getCommandArg4List() { return FunctionArg4Map.keys(); }
    QStringList getCommandArg5List() { return FunctionArg5Map.keys(); }
    QStringList getCommandArg6List() { return FunctionArg6Map.keys(); }
    QStringList getCommandArg7List() { return FunctionArg7Map.keys(); }
    QStringList getCommandArg8List() { return FunctionArg8Map.keys(); }
    QStringList getCommandArg9List() { return FunctionArg9Map.keys(); }
    QStringList getCommandArg10List() { return FunctionArg10Map.keys(); }
    QStringList getCommandArgSList() { return FunctionArgSMap.keys(); }
    QStringList getCommandArgISList() { return FunctionArgISMap.keys(); }

    /* sensor */
    QJsonArray getSensorArr() { return mSensorArr; }

    bool isResetOk();

    /* function */
    virtual bool cmd_Reset();
    bool cmd_SystemTimeSync(int year, int month, int day, int hh, int mm, int sec, int msec);
    bool cmd_SystemTimeSync();
    bool cmd_CheckSensorValue();

    bool isFuncDone(const QString &api);
    FuncResultType getFuncResult(const QString &api);

signals:
    void onFunctionFinished_signal(const QString &api, const QJsonValue &resValue);
    void onFunctionFailed_signal(const QString &api, const QJsonObject &errorObj);
    void onFunctionTimeout_signal(const QString &api, const QJsonObject &infoObj);

    void sendPageInfo_signal(const QString &dev_id, int code, const QString &message);
    void sonserInfo_signal(const QString &dev_id, const QJsonObject &obj);
    void floaterSensorInfo_signal(const QString &dev_id, const QJsonObject &obj);

public slots:
    void exeFunction_slot(const QString &api, const QJsonValue &arg);

    virtual void close_slot();

protected:
    virtual bool handleReceiveResult(const QString &api, const QJsonValue &resValue);
    virtual void handleReceiveResultError(const QString &api, const QJsonObject &errorObj);
    virtual void handleNoticification(const JPacket &p);

    /* SEND Command */
    void SendComboActionStart(const QString &api, const QJsonValue &argValue = QJsonValue(), quint32 timeout = 80000);
    void SendComboActionStop(const QString &api);
    void SendCommand(const QString &command, const QJsonValue &argValue = QJsonValue());

    /* config function map */
    void setFunctionMap(const QString &api, pFunction fn) { FunctionMap.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg1 fn) { FunctionArg1Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg2 fn) { FunctionArg2Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg3 fn) { FunctionArg3Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg4 fn) { FunctionArg4Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg5 fn) { FunctionArg5Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg6 fn) { FunctionArg6Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg7 fn) { FunctionArg7Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg8 fn) { FunctionArg8Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg9 fn) { FunctionArg9Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg10 fn) { FunctionArg10Map.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArg fn) { FunctionArgMap.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArgS fn) { FunctionArgSMap.insert(api, fn); }
    void setFunctionMap(const QString &api, pFunctionArgIS fn) { FunctionArgISMap.insert(api, fn); }

    bool m_Lock_Reset;

private:
    FMessageCenter *mMsgCenter;

    bool m_isEnable;

    /* info */
    const QString mDeviceId;
    quint16 mAddress;
    QString mUsername;

    /* config */
    QMap<QString, pFunction> FunctionMap;
    QMap<QString, pFunctionArg>     FunctionArgMap;
    QMap<QString, pFunctionArg1>    FunctionArg1Map;
    QMap<QString, pFunctionArg2>    FunctionArg2Map;
    QMap<QString, pFunctionArg3>    FunctionArg3Map;
    QMap<QString, pFunctionArg4>    FunctionArg4Map;
    QMap<QString, pFunctionArg5>    FunctionArg5Map;
    QMap<QString, pFunctionArg6>    FunctionArg6Map;
    QMap<QString, pFunctionArg7>    FunctionArg7Map;
    QMap<QString, pFunctionArg8>    FunctionArg8Map;
    QMap<QString, pFunctionArg9>    FunctionArg9Map;
    QMap<QString, pFunctionArg10>   FunctionArg10Map;
    QMap<QString, pFunctionArgS>    FunctionArgSMap;
    QMap<QString, pFunctionArgIS>   FunctionArgISMap;

    QJsonArray mSensorArr;
    QMap<QString, int> SensorPortMap;
    QMap<QString, int> FuncTimeoutMap;
    QMap<QString, int> FuntionIdMap;

    bool m_isResetting;
    bool m_isResetError;

    /* message */
    QMap<quint64, QString> m_requestMap;
    QList<JPacket> m_sendingList;

    /* send timer */
    QTimer *mSendMsgTimer;

    /* function time out */
    QMap<QString, FuncResultType> m_funcResultMap;
    QMap<quint64, QTimer*> m_funcTimeoutMap;

    /* sensor value */
    bool m_isCheckingSensor;
    QMap<QString, bool> m_sensorReqMap;
    QJsonObject m_sensorObj;

    QMap<QString, QVariant> m_floaterSensorMap;

    void setFunctionDone(const QString &api, FuncResultType result);
    bool functionContains(const QString &api);
    void clearTimeoutTimer();
    bool handleSelfWSMessage(const QString &api, const JPacket &result);

protected:
    int mBoardNum;

private slots:
    void SendPacket_slot();

    void handleCanbusMessage_slot(const JPacket &result, const JPacket &request);
    void handleCanbusNotification_slot(const JPacket &notification);
    void handleTimeoutPacket_slot();    
};

#endif // RT_DEVICE_BASE_H

