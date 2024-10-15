#ifndef RT_RACK_MANAGER_H
#define RT_RACK_MANAGER_H

#include "rt_rack.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>

class RtRackManager : public QObject
{
    Q_OBJECT
public:
    static RtRackManager *GetInstance();

    QSharedPointer<RtRack> NewRackObj();

    void removeRackOne(const QString &rid);
    void removeRackAll();

    void abort();

private:
    explicit RtRackManager(QObject *parent = nullptr);

    quint64 m_rid;

    QMap<QString, QSharedPointer<RtRack>> m_rackMap;
};

#endif // RT_RACK_MANAGER_H
