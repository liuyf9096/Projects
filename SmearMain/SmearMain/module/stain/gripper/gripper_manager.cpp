#include "gripper_manager.h"
#include "m_gripper1.h"
#include "m_gripper2.h"
#include "sql/f_sql_database_manager.h"
#include "sample/rt_sample_manager.h"

const int G_Gap = 1;

//#define DebugGripper

GripperManager *GripperManager::GetInstance()
{
    static GripperManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new GripperManager();
    }
    return instance;
}

GripperManager::GripperManager(QObject *parent)
    : QObject(parent)
{
    mCoordMap = _getCoordMap();
    qDebug() << "Gripper Coord Map:" << mCoordMap;

    mG1 = new MGripper1("gripper1", "G1", this);
    mG2 = new MGripper2("gripper2", "G2", this);
    mG1->coord.setCoordMap(mCoordMap);
    mG2->coord.setCoordMap(mCoordMap);
    mG1->resetCoord();
    mG2->resetCoord();
}

QMap<int, int> GripperManager::_getCoordMap()
{
    QMap<int, int> map;
    FSqlDatabase *db = FSqlDatabaseManager::GetInstance()->getDatebase("config");
    if (db) {
        QJsonArray arr = db->selectRecord("slot_coord");
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject obj = arr.at(i).toObject();
            int pos = obj.value("pos").toInt();
            int coord = obj.value("coord").toInt();
            map.insert(pos, coord);
        }
    }

    if (map.isEmpty()) {
        qFatal("miss slot coord data.");
    }
    return map;
}

void GripperManager::start()
{
    mG1->start();
    mG2->start();
}

void GripperManager::reset()
{
    mG1->reset();
    mG2->reset();
}

void GripperManager::stop()
{
    mG1->stop();
    mG2->stop();
}

bool GripperManager::requestMoveAccess(MGripperBase *gripper, int from, int to)
{
    int c1 = mCoordMap.value(from);
    int c2 = mCoordMap.value(to);

    if (gripper == mG1) {
        int g1_left = qMax(c1, c2) + gripper->leftOffset();
        int g2_right = mG2->coord.right;

        if (g2_right > g1_left + G_Gap) {
            mG1->coord.setRange(from, to);
            return true;
        } else {
#ifdef DebugGripper
            qDebug().noquote() << QString("Can not handle [G1] Reqest:%1->%2 Current:%3,%4")
                                  .arg(from).arg(to).arg(mG1->coord.getCoorStr(), mG2->coord.getCoorStr());
#endif
            return false;
        }
    } else if (gripper == mG2) {
        int g1_left = mG1->coord.left;
        int g2_right = qMin(c1, c2) - gripper->rightOffset();

        if (g2_right > g1_left + G_Gap) {
            mG2->coord.setRange(from, to);
            return true;
        } else {
#ifdef DebugGripper
            qDebug().noquote() << QString("Can not handle [G2] Reqest:%1->%2 Current:%3,%4")
                                  .arg(from).arg(to).arg(mG1->coord.getCoorStr(), mG2->coord.getCoorStr());
#endif
            return false;
        }
    }

    return false;
}

void GripperManager::onGripperRequest_slot(const QJsonObject &obj)
{
    if (obj.isEmpty() == false) {
        int from_pos = obj.value("from_pos").toInt();
        QString from_groupid = obj.value("from_groupid").toString();
        QString to_groupid = obj.value("to_groupid").toString();

        if (from_pos <= 26) {
            mG1->addRequest(obj);
            qDebug() << "G1 add task:" << obj;
        } else if (to_groupid == "c2" || to_groupid == "wash" || to_groupid == "recycle") {
            mG2->addRequest(obj);
            qDebug() << "G2 add task:" << obj;
        } else if (from_groupid == "transfer") {
            mG2->addRequest(obj);
            qDebug() << "G2 add task:" << obj;
        } else {
            mG2->addRequest(obj);
            qDebug() << "G2 add task:" << obj;
        }
    }
}

