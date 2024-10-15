#ifndef F_SQL_DATABASE_P_H
#define F_SQL_DATABASE_P_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>

class QThread;
class FSqlDatabase;
class FSqlDatabasePrivate : public QObject
{
    Q_OBJECT
public:
    explicit FSqlDatabasePrivate(FSqlDatabase *parent);
    virtual ~FSqlDatabasePrivate();

    bool openDatabase(const QString &dbName, const QString &connection);
    void closeDatabase();

signals:
    void selectResult_signal(const QString &key, const QJsonArray &arr);

    void insert_signal(const QString &table, const QJsonObject &valueObj);
    void update_signal(const QString &table, const QJsonObject &setObj, const QJsonObject &whereObj);
    void delete_signal(const QString &table, const QJsonObject &whereObj = QJsonObject(), bool clearSeq = true);
    void select_signal(const QString &key, const QString &table, const QJsonObject &whereObj = QJsonObject(), int limit = 0);
    void sqlQuery_signal(const QString &key, const QString &table, const QString &query);

public slots:
    bool db_insert(const QString &table, const QJsonObject &valueObj);
    bool db_update(const QString &table, const QJsonObject &setObj, const QJsonObject &whereObj);
    bool db_delete(const QString &table, const QJsonObject &whereObj = QJsonObject(), bool clearSeq = true);
    QJsonArray db_select(const QString &table, const QJsonObject &whereObj = QJsonObject(), int limit = 0);
    QJsonArray db_select(const QString &table, const QString &where, int limit = 0);
    void db_select_asyn(const QString &key, const QString &table, const QJsonObject &whereObj = QJsonObject(), int limit = 0);

private:
    FSqlDatabase * const q_ptr;
    Q_DECLARE_PUBLIC(FSqlDatabase)

    QSqlDatabase mDb;
    QThread *mThread;

    bool exeSqlQuery(const QString &query);
};

#endif // F_SQL_DATABASE_P_H
