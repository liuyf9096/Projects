#include "rt_rack_manager.h"

#include <QDebug>

RtRackManager *RtRackManager::GetInstance()
{
    static RtRackManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new RtRackManager();
    }
    return instance;
}

RtRackManager::RtRackManager(QObject *parent)
    : QObject(parent)
    , m_rid(1)
{

}

QSharedPointer<RtRack> RtRackManager::NewRackObj()
{
    QString rackid = QString("R%1").arg(m_rid);
    auto rack = QSharedPointer<RtRack>(new RtRack(rackid));

    m_rackMap.insert(rackid, rack);
    m_rid++;

    return rack;
}

void RtRackManager::removeRackOne(const QString &rid)
{
    if (m_rackMap.contains(rid)) {
        m_rackMap.remove(rid);
        qDebug() << "remove rack:" << rid;
    }
}

void RtRackManager::removeRackAll()
{
    m_rackMap.clear();
    qDebug() << "delete All racks.";
}

void RtRackManager::abort()
{
    foreach (auto rack, m_rackMap) {
        rack->abort();
    }
}
