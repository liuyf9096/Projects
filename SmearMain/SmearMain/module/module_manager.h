#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <QObject>
#include <QMap>

class MUnityTask;
class DModuleManager : public QObject
{
    Q_OBJECT
public:
    static DModuleManager *GetInstance();

    void startAutoTest();
    void suspendAutoTest();
    void stopAutoTest();
    void abortAutoTest();

    void startOne(const QString &mid);
    void reset(const QString &mid);
    void stop(const QString &mid);

    bool isRunning() { return m_isRuning; }

    MUnityTask *mUnityTask;

public slots:
    void start();
    void reset();
    void stop();

private:
    explicit DModuleManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(DModuleManager)

    bool m_isUnited;
    bool m_isRuning;

    void _init();
    void _logDbInit();
};

#endif // MODULE_MANAGER_H

