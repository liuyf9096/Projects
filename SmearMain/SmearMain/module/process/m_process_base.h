#ifndef M_PROCESS_BASE_H
#define M_PROCESS_BASE_H

#include "module_base.h"
#include "sql/f_sql_database_manager.h"

class MProcessBase : public DModuleBase
{
    Q_OBJECT
public:
    explicit MProcessBase(const QString &mid, const QString &userid, QObject *parent = nullptr);

    virtual bool startProcess();
    virtual bool startProcess(quint64 id);

    void resumeProcess();
    virtual void stopProcess(quint64 id);
    virtual void stopProcess();

    virtual void resetProcess(quint64 id);
    virtual void resetProcess();

    bool isFinished() { return m_isFinished; }
    bool isError() { return m_isError; }

    void sendFinishSignal();
    void sendErrorSignal();

signals:
    void onProcessFinished_signal(const QString &process);
    void onProcessError_signal(const QString &process);

protected:
    virtual void state_init() = 0;
    quint64 m_reqId;

    FSqlDatabase *mRecordDb;

    bool m_isFinished;
    bool m_isError;
    QTimer *mTimer;
    QMap<QString, bool> m_ProcessMap;

    bool checkStateAllFinished();

    void sendOkMessage(quint64 id);
    void sendOkMessage();
    void sendErrorMessage(quint64 id, const QString &msg = QString());
    void sendErrorMessage(const QString &msg = QString());
    void sendProcessMessage(int percent, const QString &process, const QString &message = QString());

protected slots:
    virtual void onTimer_slot() = 0;

private:
    void _init();
};

#endif // M_PROCESS_BASE_H
