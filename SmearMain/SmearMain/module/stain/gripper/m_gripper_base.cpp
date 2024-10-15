#include "m_gripper_base.h"
#include <QTimer>

GripperCoord::GripperCoord()
    : leftOffset(11)
    , rightOffset(11)
{

}

void GripperCoord::setCoord(int pos)
{
    mPos = pos;
    center = mCoordMap.value(pos);
    left = mCoordMap.value(pos) + leftOffset;
    right = mCoordMap.value(pos) - rightOffset;

    qDebug().noquote() << QString("[%1] Pos:%2  Coord:%3[%4,%5]")
                          .arg(m_name).arg(mPos).arg(center).arg(left).arg(right);
}

void GripperCoord::setOffset(int left, int right)
{
    leftOffset = left;
    rightOffset = right;

    qDebug() << "GripperCoord setOffset:" << left << right;
}

void GripperCoord::setRange(int from, int to)
{
    int c1 = mCoordMap.value(from);
    int c2 = mCoordMap.value(to);

    int left_max = qMax(c1, c2) + leftOffset;
    int right_min = qMin(c1, c2) - rightOffset;

    left = qMax(left, left_max);
    right = qMin(right, right_min);

    qDebug().noquote() << QString("[%1] Move: %2 -> %3, coord range:(%4,%5)")
                          .arg(m_name).arg(from).arg(to).arg(left).arg(right);
}

QString GripperCoord::getCoorStr()
{
    QString str = QString("(%1 [%2] %3)").arg(left).arg(center).arg(right);
    return str;
}

/*******************************************************************************/

GripperRequest::GripperRequest(const QJsonObject &obj)
    : from_pos(0)
    , to_pos(0)
{
    if (obj.contains("from_groupid")) {
        from_groupid = obj.value("from_groupid").toString();
    }
    if (obj.contains("to_groupid")) {
        to_groupid = obj.value("to_groupid").toString();
    }
    if (obj.contains("from_pos")) {
        from_pos = obj.value("from_pos").toInt();
    }
    if (obj.contains("to_pos")) {
        to_pos = obj.value("to_pos").toInt();
    }
    if (obj.contains("sid")) {
        sid = obj.value("sid").toString();
    }
}

void GripperRequest::clear()
{
    from_pos = 0;
    to_pos = 0;
    from_groupid.clear();
    to_groupid.clear();
    sid.clear();
}

bool GripperRequest::isEmpty()
{
    if (from_pos != 0 || to_pos != 0) {
        return false;
    }
    if (from_groupid.isEmpty() == false || to_groupid.isEmpty() == false || sid.isEmpty() == false) {
        return false;
    }
    return true;
}

bool GripperRequest::operator==(const GripperRequest &other) const
{
    if (other.from_groupid == from_groupid && other.to_groupid == to_groupid) {
        if (other.from_pos == from_pos && other.to_pos == to_pos) {
            if (other.sid == sid) {
                return true;
            }
        }
    }
    return false;
}

/*******************************************************************************/

MGripperBase::MGripperBase(const QString &mid, const QString &userid, QObject *parent)
    : DModuleBase(mid, userid, parent)
{
    mTimer = new QTimer(this);
    mTimer->setInterval(50);
    connect(mTimer, &QTimer::timeout, this, &MGripperBase::onTaskTimer_slot);
}

void MGripperBase::start()
{
    mTimer->start();
    DModuleBase::start();
}

void MGripperBase::reset()
{
    state_init();
    DModuleBase::reset();
}

void MGripperBase::stop()
{
    mTimer->stop();
    DModuleBase::stop();
}

void MGripperBase::addRequest(const QJsonObject &obj)
{
    GripperRequest req(obj);
    if (!req.isEmpty()) {
        m_requestList.append(req);

        if (mTimer->isActive() == false) {
            mTimer->start();
        }
    }
}

