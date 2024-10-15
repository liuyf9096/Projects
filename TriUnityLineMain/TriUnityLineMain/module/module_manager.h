#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <QObject>
#include <QMap>

class MImport;
class MBootProcess;
class MAgingTestProcess;
class DModuleManager : public QObject
{
    Q_OBJECT
public:
    static DModuleManager *GetInstance();

    MImport *mImport;
    MBootProcess *mBoot;
    MAgingTestProcess *mAgingTest;

    void start();
    void reset();
    void stop();

private:
    explicit DModuleManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(DModuleManager)

    void _init();
};

#endif // MODULE_MANAGER_H
