#ifndef EXCEPTION_CENTER_H
#define EXCEPTION_CENTER_H

#include <QObject>
#include <QMap>

enum class Exception_Type {
    Unknown,
    Action_Fail,
    Action_Timeout,
    Consume_Shortage
};

enum class E_Level {
    Unknown,
    Ignorable,
    Alarm,
    Error
};

class Exception {
public:
    explicit Exception(const QString &error_id, E_Level level = E_Level::Alarm, int code = 0);

    QString e_id;
    int e_code;
    Exception_Type e_type;
    E_Level e_level;
    QString e_module;
    QString e_msg;
};

class ExceptionCenter : public QObject
{
    Q_OBJECT
public:
    static ExceptionCenter *GetInstance();
    static QString getItemTypeS(int itemType);
    static QString getErrorTypeS(quint8 itemType, quint8 errorType);

    void addException(const QString &error_id, Exception exp);
    void removeException(const QString &error_id, bool isSendUI = false);
    void sendExceptionMessage(const Exception &exp);
//    void sendExceptionObj(const QString &error_id, const QJsonObject &obj);
    void sendClearExceptionMessage(const QString &error_id);

    void handleClearException(const QString &error_id, quint64 id);

    bool isAlarmException();
    bool isErrorException();

public slots:
    void onFunctionFinished_slot(const QString &api, const QJsonValue &);
    void onFunctionFailed_slot(const QString &api, const QJsonObject &errorObj);
    void onFunctionTimeout_slot(const QString &api, const QJsonObject &infoObj);

private:
    explicit ExceptionCenter(QObject *parent = nullptr);

    QMap<QString, Exception> m_ExpMap;
};

#endif // EXCEPTION_CENTER_H
