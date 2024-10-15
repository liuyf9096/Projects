#ifndef M_GRIPPER1_H
#define M_GRIPPER1_H

#include "m_gripper_base.h"

class MGripper1 : public MGripperBase
{
    Q_OBJECT
public:
    explicit MGripper1(const QString &mid, const QString &userid, QObject *parent = nullptr);

    void resetCoord();

    virtual bool cmd_Gripper(int from, int to, QString &errorid) override;

protected:
    virtual void state_init() override;
    void onGripperStatusInfo_slot(const QJsonObject &obj);

protected slots:
    virtual void onTaskTimer_slot() override;

private:
    void _init();
    void _getOffset();

    int mImportPos;
    int m_gripperOpenCount;
};

#endif // M_GRIPPER1_H
