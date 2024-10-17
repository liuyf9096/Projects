#ifndef MOBDEBUG_H
#define MOBDEBUG_H

#include <QObject>
#include <QProcess>
#include <QUdpSocket>
#include <QDir>

class Mobdebug : public QObject
{
    Q_OBJECT
public:
    enum ModebugState {
        MODEBUG_IDLE,
        MODEBUG_STARTED,
        MODEBUG_LISTENING,
        MODEBUG_CONNECTED,
        MODEBUG_RUNNING,
        MODEBUG_SUSPENDED,
        MODEBUG_STOP
    };

    explicit Mobdebug(QObject *parent = nullptr);

    bool listen(quint64 id = 0);

    void udpOpen();
    void udpClose();

    void mo_run(quint64 id = 0);                            // 继续执行目标程序
    void mo_suspend(quint64 id = 0);                        // debug break
    void mo_step(quint64 id = 0);                           // step into
    void mo_over(quint64 id = 0);                           // step over
    void mo_eval(QString exp, quint64 id = 0);              // 对表达式求值
    void mo_exec(QString stmt, quint64 id = 0);             // 对表达式求值
    void mo_setb(QString file, int line, quint64 id = 0);   // 设置断点
    void mo_delb(QString file, int line, quint64 id = 0);   // 删除断点
    void mo_delallb(quint64 id = 0);                        // 删除所有断点
    void mo_stack(quint64 id = 0);                          // 查询当前Lua执行线
    void mo_help(quint64 id = 0);                           // 帮助指令
    void mo_exit(quint64 id = 0);                           // 退出调试

signals:
    void onModebugStateChanged_signal(ModebugState state, quint64 id = 0);
    void onModebugMessage_signal(QString msg);

    void onFinish_signal(quint64 id);
    void onErrorOccured_signal(quint64 id, int code);

private:
    bool isBusy;
    quint64 m_handlingId;

    ModebugState m_state;
    QProcess *m_process;

    QUdpSocket *udpPrint;
    QUdpSocket *udp1Cursor;
    QUdpSocket *udp2Cursor;
    QUdpSocket *udpSpecial;

    void _processInit();
    void _udpInit();

    bool _processWrite(QString cmd);

private slots:
    void onProcessStarted_slot();
    void onProcessReadyRead_slot();
    void onProcessErrorOccurred_slot(QProcess::ProcessError error);
    void onProcessFinished_slot(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessStateChanged(QProcess::ProcessState newState);

    void readPendingDatagrams_slot();
};

#endif // MOBDEBUG_H
