#ifndef GRIPPER_MANAGER_H
#define GRIPPER_MANAGER_H

#include <QObject>
#include <QMap>

class MGripperBase;
class MGripper1;
class MGripper2;
class GripperManager : public QObject
{
    Q_OBJECT
public:
    static GripperManager *GetInstance();

    void start();
    void reset();
    void stop();

    MGripper1 *mG1;
    MGripper2 *mG2;

    bool requestMoveAccess(MGripperBase *gripper, int from, int to);

public slots:
    void onGripperRequest_slot(const QJsonObject &obj);

private:
    explicit GripperManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(GripperManager)

    QMap<int, int> mCoordMap;
    QMap<int, int> _getCoordMap();
};

#endif // GRIPPER_MANAGER_H
