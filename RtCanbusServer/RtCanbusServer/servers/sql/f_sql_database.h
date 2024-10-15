#ifndef F_SQL_DATABASE_H
#define F_SQL_DATABASE_H

/***************************************************
 * function : used for SQL Simple operation        *
 *   author : liuyufei                             *
 * datetime : 2021-11-25 10:52:50                  *
 *  version : 2.0                                  *
 ***************************************************/

#include <QObject>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

class FSqlDatabasePrivate;
class FSqlDatabase : public QObject
{
    Q_OBJECT
public:
    explicit FSqlDatabase(QObject *parent = nullptr);

    bool openDatabase(const QString &dbFileName, const QString &connection);
    void closeDatabase();

    /* basic */
    bool insertRecord(const QString &table, const QJsonObject &valueObj);
    bool updateRecord(const QString &table, const QJsonObject &setObj, const QJsonObject &whereObj);
    bool deleteRecord(const QString &table, const QJsonObject &whereObj = QJsonObject(), bool clearSeq = true);
    QJsonArray selectRecord(const QString &table, const QJsonObject &whereObj = QJsonObject(), int limit = 0);
    QJsonArray selectRecord(const QString &table, const QString &where, int limit = 0);
    void selectRecord_asyn(const QString &key, const QString &table, const QJsonObject &whereObj = QJsonObject(), int limit = 0);

signals:
    void selectResult_signal(const QString &key, const QJsonArray &arr);

private:
    FSqlDatabasePrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FSqlDatabase)
};

#endif // F_SQL_DATABASE_H
