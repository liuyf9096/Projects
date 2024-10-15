#ifndef EXCEPTION_CENTER_H
#define EXCEPTION_CENTER_H

#include <QObject>
#include <QMap>
#include <QJsonObject>

enum class Exception_Type {
    Unknown,
    Action_Fail,
    Action_Timeout,
    UserCode
};

enum class E_Level {
    Unknown,
    Ignorable,
    Alarm,
    Error
};

class FSqlDatabase;
class Exception {
public:
    explicit Exception(const QString &error_id, E_Level level = E_Level::Alarm, int code = 0);

    QString e_module;
    QString e_id;
    QString e_msg;
    int e_code;
    Exception_Type e_type;
    E_Level e_level;
};

class ExceptionCenter : public QObject
{
    Q_OBJECT
public:
    static ExceptionCenter *GetInstance();
    static QString getItemTypeS(int itemType);
    static QString getErrorTypeS(quint8 itemType, quint8 errorType);

    void addException(const QString &dev_id, const QString &api, Exception_Type type, const QString &usercode = QString());
    void sendExceptionMessage(QJsonObject obj);

    void removeException(const QString &error_id);
    void removeException(const QString &device, const QString &api);
    void sendRemoveExceptionMessage(const QString &error_id);
    void sendRemoveExceptionMessages(const QStringList &list);

    QJsonArray getAllException();

public slots:
    void onFunctionFinished_slot(const QString &api, const QJsonValue &);
    void onFunctionFailed_slot(const QString &api, const QJsonObject &errorObj);
    void onFunctionTimeout_slot(const QString &api);

private:
    explicit ExceptionCenter(QObject *parent = nullptr);

    FSqlDatabase *mLogDb;
    FSqlDatabase *mConfigDb;

    QMap<QString, QJsonObject> m_errorMap;
    QMap<QString, Exception> m_ExpMap;
};

#endif // EXCEPTION_CENTER_H
