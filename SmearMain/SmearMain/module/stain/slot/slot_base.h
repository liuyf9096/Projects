#ifndef SLOTBASE_H
#define SLOTBASE_H

#include "sample/rt_sample.h"
#include "module_base.h"

enum Detergent {
    Detergent_Water,
    Detergent_Alcohol
};

class FSqlDatabase;
class QTimer;
class SlotBase : public DModuleBase
{
    friend class SolutionInfuser;
    friend class SolutionDrainer;

    Q_OBJECT
public:
    explicit SlotBase(int pos, const QString &group, QObject *parent = nullptr);

    const int mPos;

    QSharedPointer<RtSlide> slide() { return m_slide; }

    virtual bool isAvailable();
    virtual void prepareSolution(const QString &) {}

    void setDuration(int sec);
    int duration() { return mDurationSec; }
    void setStainMethod(const QString &method) { m_stainMethod = method; }

    virtual void setMaxStainCount(int) {}
    virtual void setSolutionExpiryTime(int) {}

    void setWaterSoakTime(int sec) { m_water_soaktime_s = sec; }
    void setAlcoholSoakTime(int sec) { m_alcohol_soaktime_s = sec; }

    void setRemainSlideSlot(const QString &slide_id);

    void putinSlide(const QString &sid);
    void takeoutSlide(const QString &sid);

signals:
    void onGripperRequest_signal(const QJsonObject &obj);

protected:
    QString mGroup;
    QString mSlotInfo;
    quint64 m_request_id;
    QString m_prepare_slideid;

    bool m_isAutoSolutionFill;
    bool m_isSolutionFilled;

    bool m_isCleaning;
    bool m_isDrained;
    bool m_isDetergentFilled;
    bool m_isClean;

    int mMaxStainCount;
    int m_stainCount;
    int mExpiryTimeSec; //sec
    int m_solutionDurationSec;

    int m_water_soaktime_s;
    int m_alcohol_soaktime_s;

    QSharedPointer<RtSlide> m_slide;

    QString m_stainMethod;

    int mDurationSec;

    virtual void handlePutinSlide() {}
    virtual void handleTakeoutSlide() {}

    QTimer *m_durationTimer;
    virtual void onDurationTimeout() {}

protected slots:
    void onDurationTimeout_slot();
};

#endif // SLOTBASE_H
