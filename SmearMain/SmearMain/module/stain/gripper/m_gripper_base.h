#ifndef M_GRIPPER_BASE_H
#define M_GRIPPER_BASE_H

#include "module_base.h"
#include <QList>

class GripperCoord
{
public:
    GripperCoord();

    void setName(const QString &name) { m_name = name; }
    void setCoord(int pos);

    void setOffset(int left, int right);
    void setCoordMap(QMap<int, int> map) { mCoordMap = map; }

    void setRange(int from, int to);
    QString getCoorStr();

    int mPos{0};
    int center{0};
    int left{0};
    int right{0};

private:
    QString m_name;

    int leftOffset;
    int rightOffset;

    void _getCoordMap();

    QMap<int, int> mCoordMap;
};

class GripperRequest
{
public:
    GripperRequest(const QJsonObject &obj = QJsonObject());

    void clear();
    bool isEmpty();

    QString from_groupid;
    QString to_groupid;
    int from_pos;
    int to_pos;
    QString sid;

    bool operator==(const GripperRequest &other) const;
};

class MGripperBase : public DModuleBase
{
    Q_OBJECT
public:
    explicit MGripperBase(const QString &mid, const QString &userid, QObject *parent = nullptr);

    int resetPos() { return mResetPos; }
    int avoidPos() { return mAvoidPos; }
    int pos() { return m_pos; }

    int leftOffset() { return mLeftOffset; }
    int rightOffset() { return mRightOffset; }

    void clearErrorid() { m_errorid.clear(); }

    virtual void start() override;
    virtual void reset() override;
    virtual void stop() override;

    void addRequest(const QJsonObject &obj);

    GripperCoord coord;

protected:
    int mResetPos;
    int mAvoidPos;

    int mRecyclePos;

    int mLeftOffset;
    int mRightOffset;

    int m_pos;
    int m_toPos;

    QString m_api;
    QList<GripperRequest> m_requestList;
    GripperRequest m_Req;
    QString m_errorid;

    virtual void state_init() = 0;
    virtual bool cmd_Gripper(int from, int to, QString &errorid) = 0;

    enum Process {
        Idle,
        Send_Trasnfer_Cmd,
        WaitF_Transfer_Cmd_Done,
        Finish,
        Error
    } s_process;

protected slots:
    virtual void onTaskTimer_slot() = 0;

private:
    QTimer *mTimer;
};

#endif // M_GRIPPER_BASE_H
