#ifndef DROBOTDEBUGGER_H
#define DROBOTDEBUGGER_H

#include <QObject>
#include <QTime>
#include <QFileInfo>
#include <QProcess>
#include <QUdpSocket>

class DRobotDebugger : public QObject
{
    Q_OBJECT

public:
    explicit DRobotDebugger(QObject *parent = nullptr);
    ~DRobotDebugger();

public:
    enum DebugState{
        MobDebugStarting = 0,
        MobDebugStarted,
        SrcRunning,
        SrcSuspend,
        SrcFinished,
        MobDebugStop
    };

    void sleep(unsigned int msec);
    DebugState debugState(void);

    QString SendCmd(QString cmd);

    void onClientMsg(QString msg);
    void on1stCursorMsg(QString msg);
    void on2ndCursorMsg(QString msg);
    void onSpecialMsg(QString msg);


signals:
    void debugStateChanged(DebugState state);
    void debugStateEnabled(bool isEnabled);
    void stopStateEnabled(bool isEnabled);
    void stepInChanged(bool isStep, bool isStart, bool isDebug);

    void mobDebugInfo(QString str);

    void runningLineChanged(QString fileName, int linNumber);
    void errorLine(int line);
    void errorMsg(QString msg);
    void scriptLineChanged(int line);
    void moveLineChanged(int line);

    void printQRobotDebugger(QString str);

public slots:
    void start(const QString &fileName);
    void stop(void);
    void runScript(void);
    void suspend(void);
    void suspendForAPI(void);
    bool getSuspend(void);
    void stepIn(void);
    void stepOver(void);
    bool getStepIn(void);
    QString getGlobalVal(QString var);
    QString changeGlobalVal(QString variable, QString value);

    void setBreakpoint(const QString &fileName, int lineNumber);
    void removeBreakpoint(const QString &fileName, int lineNumber);
    void removeAllBreakpoints(void);

private:

    QUdpSocket *udpPrint;
    QUdpSocket *udp1Cursor;
    QUdpSocket *udp2Cursor;
    QUdpSocket *udpSpecial;

    QProcess *m_process;

    inline bool checkProcLife(void);
    inline void stopMobDebug(void);

    //进程同步通讯
    inline QString writeSync(const QString &sendMsg);

    bool m_isStepping;
    bool m_isStepFinished;
    bool m_hasError;
    QTime m_timeout;
    DebugState m_state;
};

#endif // DROBOTDEBUGGER_H
