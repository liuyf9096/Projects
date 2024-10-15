#ifndef F_SQL_DATABASE_MANAGER_H
#define F_SQL_DATABASE_MANAGER_H

#include "f_sql_database.h"

#include <QDateTime>
#include <QObject>
#include <QMap>

class FSqlDatabaseManager : public QObject
{
    Q_OBJECT
public:
    static FSqlDatabaseManager *GetInstance();

    void addDatabase(const QString &dbFileName, const QString &connection);
    FSqlDatabase *getDatebase(const QString &connection);

private:
    explicit FSqlDatabaseManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(FSqlDatabaseManager)

    QMap<QString, FSqlDatabase*> m_sqlDbMap;
};

#endif // F_SQL_DATABASE_MANAGER_H
