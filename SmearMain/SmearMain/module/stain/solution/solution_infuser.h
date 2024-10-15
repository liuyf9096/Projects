#ifndef SOLUTION_INFUSER_H
#define SOLUTION_INFUSER_H

#include "module_base.h"

class SolutionInfuser : public DModuleBase
{
    Q_OBJECT
public:
    explicit SolutionInfuser(const QString &mid, QObject *parent = nullptr);

    void addRequest(const QString &solution, int pos, int pa = 0, int pb = 0);
    void addDetergentRequest(const QString &solution, int pos);
    void addDetergentRequest(const QString &solution, QList<int> posList);
    bool isIdle() { return m_requestObjList.isEmpty(); }

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

private:
    void state_init();
    bool cmd_FillSolution(const QString &solution, int pos, int pa = 0, int pb = 0);
    void handleSlotState(const QString &api, int pos);

    QTimer *mTimer;
    QJsonObject m_taskObj;
    QString m_api;
    int m_handlingPos;
    QString m_solution;

    enum Process {
        Idle,
        Send_Fill_Cmd,
        WaitF_Fill_Cmd_Done,
    } m_state;

    QList<QJsonObject> m_requestObjList;

private slots:
    void onTaskTimer_slot();
};

#endif // SOLUTION_INFUSER_H
