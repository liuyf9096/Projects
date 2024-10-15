#include "f_sql_database.h"
#include "f_sql_database_p.h"

#define MoveToThread

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QThread>
#include <QDebug>

FSqlDatabase::FSqlDatabase(QObject *parent)
    : QObject(parent)
    , Dptr(new FSqlDatabasePrivate(this))
{
    connect(Dptr, &FSqlDatabasePrivate::selectResult_signal,
            this, &FSqlDatabase::selectResult_signal);
}

bool FSqlDatabase::openDatabase(const QString &dbFileName, const QString &connection)
{
    Q_D(FSqlDatabase);
    return d->openDatabase(dbFileName, connection);
}

void FSqlDatabase::closeDatabase()
{
    Q_D(FSqlDatabase);
    d->closeDatabase();
}

int FSqlDatabase::count(const QString &table)
{
    Q_D(FSqlDatabase);
    return d->db_count(table);
}

/* "INSERT INTO xxdb.student (id, name) VALUES (5, '小明');" */
bool FSqlDatabase::insertRecord(const QString &table, const QJsonObject &valueObj)
{
    Q_D(FSqlDatabase);
#ifdef MoveToThread
    emit d->insert_signal(table, valueObj);
    return true;
#else
    return d->db_insert(table, valueObj);
#endif
}

/* "UPDATE xxdb.student SET age=16 WHERE id=8;" */
bool FSqlDatabase::updateRecord(const QString &table, const QJsonObject &setObj, const QJsonObject &whereObj)
{
    Q_D(FSqlDatabase);
#ifdef MoveToThread
    emit d->update_signal(table, setObj, whereObj);
    return true;
#else
    return d->db_update(table, setObj, whereObj);
#endif
}

/* "DELETE FROM xxdb.test_student WHERE id=23;" */
bool FSqlDatabase::deleteRecord(const QString &table, const QJsonObject &whereObj, bool clearSeq)
{
    Q_D(FSqlDatabase);
#ifdef MoveToThread
    emit d->delete_signal(table, whereObj, clearSeq);
    return true;
#else
    return d->db_delete(table, whereObj, clearSeq);
#endif
}

/* "SELECT * FROM xxdb.student WHERE id=0 LIMIT 1000;" */
QJsonArray FSqlDatabase::selectRecord(const QString &table, const QJsonObject &whereObj, int limit)
{
    Q_D(FSqlDatabase);
    return d->db_select(table, whereObj, limit);
}

QJsonArray FSqlDatabase::selectRecord(const QString &table, const QString &where, int limit)
{
    Q_D(FSqlDatabase);
    return d->db_select(table, where, limit);
}

void FSqlDatabase::selectRecord_asyn(const QString &key, const QString &table, const QJsonObject &whereObj, int limit)
{
    Q_D(FSqlDatabase);
    emit d->select_signal(key, table, whereObj, limit);
}

FSqlDatabasePrivate::FSqlDatabasePrivate(FSqlDatabase *parent) : q_ptr(parent)
{
#ifdef MoveToThread
    connect(this, &FSqlDatabasePrivate::insert_signal,
            this, &FSqlDatabasePrivate::db_insert, Qt::QueuedConnection);
    connect(this, &FSqlDatabasePrivate::update_signal,
            this, &FSqlDatabasePrivate::db_update, Qt::QueuedConnection);
    connect(this, &FSqlDatabasePrivate::delete_signal,
            this, &FSqlDatabasePrivate::db_delete, Qt::QueuedConnection);
    connect(this, &FSqlDatabasePrivate::select_signal,
            this, &FSqlDatabasePrivate::db_select_asyn, Qt::QueuedConnection);

    mThread = new QThread();
    connect(mThread, &QThread::finished, mThread, &QThread::deleteLater);
    this->moveToThread(mThread);
#endif
}

FSqlDatabasePrivate::~FSqlDatabasePrivate()
{
#ifdef MoveToThread
    mThread->quit();
    mThread->wait();
    if (mThread->isFinished()) {
        qInfo() << "FSqlDatabasePrivate thread quit finished.";
    } else {
        qWarning() << "FSqlDatabasePrivate thread can not quit. timeout.";
    }
#endif
}

bool FSqlDatabasePrivate::openDatabase(const QString &dbName, const QString &connection)
{
    mDb = QSqlDatabase::addDatabase("QSQLITE", connection);
    mDb.setDatabaseName(dbName);
    bool ok = mDb.open();
    if (ok) {
#ifdef MoveToThread
        mThread->start();
#endif
        qDebug() << "SQL: open database:" << dbName << "OK.";
    } else {
        qCritical() << "SQL: Error, Failed to connect database:" << dbName;
    }
    return ok;
}

void FSqlDatabasePrivate::closeDatabase()
{
    if (mDb.isOpen()) {
        mDb.close();

#ifdef MoveToThread
        mThread->quit();
#endif
        qDebug().noquote() << QString("SQL: Db connection[%1] closed.").arg(mDb.connectionName());
    }
}

int FSqlDatabasePrivate::db_count(const QString &table)
{
    int count = 0;
    QString queryStr = QString("SELECT count(*) FROM %1").arg(table);
    QSqlQuery sqlQuery(mDb);
    bool ok = sqlQuery.exec(queryStr);
    if (ok && sqlQuery.first()) {
        count = sqlQuery.value(0).toInt();
    }
    return count;
}

bool FSqlDatabasePrivate::db_insert(const QString &table, const QJsonObject &valueObj)
{
    if (valueObj.isEmpty()) {
        qWarning() << "SQL: db_insert Error, value is Empty.";
        return false;
    }

    QString queryStr = QString("INSERT INTO %1").arg(table);

    queryStr.append("(");
    QStringList keys = valueObj.keys();
    for (int i = 0; i < keys.count(); ++i) {
        queryStr.append(QString("%1, ").arg(keys.at(i)));
    }
    queryStr = queryStr.left(queryStr.count() - 2);
    queryStr.append(")");

    queryStr.append(" VALUES(");
    for (int i = 0; i < keys.count(); ++i) {
        QString key = keys.at(i);
        QVariant value = valueObj.value(key).toVariant();
        if (value.type() == QVariant::Double) {
            queryStr.append(QString("%1").arg(value.toDouble()));
        } else if (value.type() == QVariant::Bool) {
            queryStr.append(QString("%1").arg(value.toBool() ? 1 : 0));
        } else {
            queryStr.append(QString("'%1'").arg(value.toString()));
        }
        queryStr.append(", ");
    }
    if (queryStr.endsWith(", ")) {
        queryStr = queryStr.left(queryStr.count() - 2);
    }
    queryStr.append(")");

    return exeSqlQuery(queryStr);
}

bool FSqlDatabasePrivate::db_update(const QString &table, const QJsonObject &setObj, const QJsonObject &whereObj)
{
    if (setObj.isEmpty()) {
        qWarning() << "SQL: db_update Error, setObj is Empty.";
        return false;
    }

    QString queryStr = QString("UPDATE %1").arg(table);

    queryStr.append(" SET ");
    QStringList set_keys = setObj.keys();
    for (int i = 0; i < set_keys.count(); ++i) {
        QString key = set_keys.at(i);
        QVariant value = setObj.value(key).toVariant();
        if (value.type() == QVariant::Bool) {
            queryStr.append(QString("%1=%2").arg(key).arg(value.toBool() ? 1 : 0));
        } else if (value.type() == QVariant::Double) {
            queryStr.append(QString("%1=%2").arg(key).arg(value.toDouble()));
        } else {
            queryStr.append(QString("%1='%2'").arg(key, value.toString()));
        }
        queryStr.append(", ");
    }
    if (queryStr.endsWith(", ")) {
        queryStr = queryStr.left(queryStr.count() - 2);
    }

    queryStr.append(" WHERE ");
    QStringList where_keys = whereObj.keys();
    for (int i = 0; i < where_keys.count(); ++i) {
        QString key = where_keys.at(i);
        QVariant value = whereObj.value(key).toVariant();
        if (value.type() == QVariant::Bool) {
            queryStr.append(QString("%1=%2").arg(key).arg(value.toBool() ? 1 : 0));
        } else if (value.type() == QVariant::Double) {
            queryStr.append(QString("%1=%2").arg(key).arg(value.toDouble()));
        } else {
            queryStr.append(QString("%1='%2'").arg(key, value.toString()));
        }
        queryStr.append(" AND ");
    }
    if (queryStr.endsWith(" AND ")) {
        queryStr = queryStr.left(queryStr.count() - 5);
    }
    queryStr.append(";");

    return exeSqlQuery(queryStr);
}

bool FSqlDatabasePrivate::db_delete(const QString &table, const QJsonObject &whereObj, bool clearSeq)
{
    QString queryStr = QString("DELETE FROM %1").arg(table);

    QStringList where_keys = whereObj.keys();
    if (!whereObj.isEmpty())
    {
        queryStr.append(" WHERE ");
        for (int i = 0; i < where_keys.count(); ++i)
        {
            QString key = where_keys.at(i);
            QVariant value = whereObj.value(key).toVariant();
            if (value.type() == QVariant::Bool) {
                queryStr.append(QString("%1=%2").arg(key).arg(value.toBool() ? 1 : 0));
            } else if (value.type() == QVariant::Double) {
                queryStr.append(QString("%1=%2").arg(key).arg(value.toDouble()));
            } else {
                queryStr.append(QString("%1='%2'").arg(key, value.toString()));
            }
            queryStr.append(" AND ");
        }
        if (queryStr.endsWith(" AND ")) {
            queryStr = queryStr.left(queryStr.count() - 5);
        }
        return exeSqlQuery(queryStr);
    } else {
        bool ok = exeSqlQuery(queryStr);
        if (ok == true && clearSeq == true) {
            QString query_2 = QString("update sqlite_sequence SET seq=0 where name='%1'").arg(table);
            ok = exeSqlQuery(query_2);
        }
        return ok;
    }
}

QJsonArray FSqlDatabasePrivate::db_select(const QString &table, const QJsonObject &whereObj, int limit)
{
    QString where;
    if (!whereObj.isEmpty()) {
        QStringList where_keys = whereObj.keys();
        for (int i = 0; i < where_keys.count(); ++i) {
            QString key = where_keys.at(i);
            QVariant value = whereObj.value(key).toVariant();
            if (value.type() == QVariant::Bool) {
                where.append(QString("%1=%2").arg(key).arg(value.toBool() ? 1 : 0));
            } else if (value.type() == QVariant::Double) {
                where.append(QString("%1=%2").arg(key).arg(value.toDouble()));
            } else {
                where.append(QString("%1='%2'").arg(key, value.toString()));
            }
            where.append(" AND ");
        }
        if (where.endsWith(" AND ")) {
            where = where.left(where.count() - 5);
        }
    }
    return db_select(table, where, limit);
}

QJsonArray FSqlDatabasePrivate::db_select(const QString &table, const QString &where, int limit)
{
    QString queryStr = QString("SELECT * FROM %1").arg(table);

    if (!where.isEmpty()) {
        queryStr.append(" WHERE ");
        queryStr.append(where);
    }
    if (limit > 0) {
        queryStr.append(QString(" LIMIT %1").arg(limit));
    }
    queryStr.append(";");

    QJsonArray jsonArr;
    QSqlQuery sqlQuery(mDb);
    sqlQuery.setForwardOnly(true);
    bool ok = sqlQuery.exec(queryStr);
    if (ok) {
        QSqlRecord record = sqlQuery.record();
        while (sqlQuery.next()) {
            QJsonObject obj;
            for (int i = 0; i < record.count(); ++i) {
                QString key = record.fieldName(i);
                obj.insert(key, sqlQuery.value(key).toJsonValue());
            }
            jsonArr.append(obj);
        }
    } else {
        qWarning() << sqlQuery.lastError().text();
    }
    return jsonArr;
}

void FSqlDatabasePrivate::db_select_asyn(const QString &key, const QString &table, const QJsonObject &whereObj, int limit)
{
    auto arr = db_select(table, whereObj, limit);
    emit selectResult_signal(key, arr);
}

bool FSqlDatabasePrivate::exeSqlQuery(const QString &query)
{
    QSqlQuery sqlquery(mDb);
    bool ok = sqlquery.exec(query);
    if (ok == false) {
        qWarning() << "SQL: exe sql False." << query;
    }
    return ok;
}
