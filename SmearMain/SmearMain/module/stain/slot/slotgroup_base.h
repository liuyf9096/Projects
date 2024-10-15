#ifndef SLOTGROUPBASE_H
#define SLOTGROUPBASE_H

#include "slot_base.h"

#include <QObject>

class SlotGroupBase : public QObject
{
    Q_OBJECT
public:
    explicit SlotGroupBase(const QString &group_id, QObject *parent = nullptr);

    void setSlot(int from, int to);
    bool contains(int pos) { return mSlotMap.contains(pos); }
    void prepareSlotPos(int pos, const QString &sid);

    virtual int getVacantSlot();
    virtual void setDuration(int sec);

    int duration() { return mDuration; }
    QString groupid() { return mGroupID; }

    virtual void stainStart(const QString &sid, int pos);
    virtual void takeOutSample(const QString &sid, int pos);

    void setStainMethod(const QString &method);
    void setRemainSlideSlot(int pos, const QString &slide_id);

    void setMaxStainCount(int stainCount);
    void setSolutionExpiryTime(int min);

    virtual void drainAllSlots(quint64) {}
    virtual void drainSlots(QVector<int>, quint64) {}
    virtual bool isDrainFinished() { return false; }

    virtual void cleanAllSlots(Detergent, quint64) {}
    virtual void cleanSlots(QVector<int>, Detergent, quint64) {}
    virtual bool isCleanFinished() { return false; }

    SlotBase *slotAt(int pos);

signals:
    void onGripperRequest_signal(const QJsonObject &obj);

protected:
    virtual void slotinit(int pos) = 0;

    QList<SlotBase *> mSlotList;
    const QString mGroupID;
    QMap<int, QString> m_posSampleMap;
    QMap<int, SlotBase *> mSlotMap;

    int mDuration;
    quint64 m_request_id;
    int mMaxStainCount;
    int m_water_soaktime_s;
    int m_alcohol_soaktime_s;
};

#endif // SLOTGROUPBASE_H
