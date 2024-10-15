#include "f_sql_database_manager.h"

#include <QDebug>

FSqlDatabaseManager *FSqlDatabaseManager::GetInstance()
{
    static FSqlDatabaseManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new FSqlDatabaseManager();
    }
    return instance;
}

FSqlDatabaseManager::FSqlDatabaseManager(QObject *parent)
    : QObject{parent}
{

}

void FSqlDatabaseManager::addDatabase(const QString &dbFileName, const QString &connection)
{
    auto database = new FSqlDatabase(this);
    bool ok = database->openDatabase(dbFileName, connection);
    if (ok) {
        m_sqlDbMap.insert(connection, database);
    } else {
        qWarning() << "SQL: open Database FALSE. db:" << dbFileName;
    }
}

FSqlDatabase *FSqlDatabaseManager::getDatebase(const QString &connection)
{
    if (m_sqlDbMap.contains(connection)) {
        return m_sqlDbMap.value(connection);
    }
    return nullptr;
}
