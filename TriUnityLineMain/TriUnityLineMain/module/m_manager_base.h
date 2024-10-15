#ifndef M_MANAGER_BASE_H
#define M_MANAGER_BASE_H

#include <QObject>
#include <QMap>

class DModuleBase;
class MManagerBase : public QObject
{
    Q_OBJECT
public:
    explicit MManagerBase(const QString &managerName, QObject *parent = nullptr);

    const QString mManagerName;

    virtual void start();
    virtual void reset();
    virtual void stop();

    QStringList getModuleList() { return ModuleMap.keys(); }

signals:
    void onDisplayMessage_signal(const QString &mid, const QJsonObject &obj);

protected:
    QMap<QString, DModuleBase*> ModuleMap;

    void addModule(DModuleBase *module);
    void addModule(DModuleBase *m1, DModuleBase *m2);
    void addModule(DModuleBase *m1, DModuleBase *m2, DModuleBase *m3);
};

#endif // M_MANAGER_BASE_H
