#ifndef M_GRIPPER2_H
#define M_GRIPPER2_H

#include "m_gripper_base.h"

class MRecycleBoxMgr;
class RecycleBox;
class MGripper2 : public MGripperBase
{
    Q_OBJECT
public:
    explicit MGripper2(const QString &mid, const QString &userid, QObject *parent = nullptr);

    void setRecycleBoxMgr(MRecycleBoxMgr *mgr) { RecycleBoxMgr = mgr; }
    void resetCoord();

    virtual bool cmd_Gripper(int from, int to, QString &errorid) override;

protected:
    virtual void state_init() override;
    void onGripperStatusInfo_slot(const QJsonObject &obj);

protected slots:
    virtual void onTaskTimer_slot() override;

private:
    MRecycleBoxMgr *RecycleBoxMgr = nullptr;
    QSharedPointer<RecycleBox> mRecycleBox = nullptr;

    void _init();
    void _getOffset();

    int m_gripperOpenCount;
};

#endif // M_GRIPPER2_H
