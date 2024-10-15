#ifndef STAINONLY_BASE_H
#define STAINONLY_BASE_H

#include "module_base.h"

class StainOnlyBase : public DModuleBase
{
    Q_OBJECT
public:
    explicit StainOnlyBase(const QString &mid, const QString &userid, int from, int to, QObject *parent = nullptr);

    bool isEmpty();

    bool hasRequest();
    bool loadSlides(quint64 id, const QJsonArray &arr);
    void unloadAllSlides();

    void sendRequest();
    bool takeOutSample(const QString &sid, int pos);

signals:
    void onSendRequest_singal(const QString &from_groupid, int from_pos, const QString &sid);
    void onTakeOutOfPos_signal(int pos, const QString &sid);

protected:
    virtual void setLightRun() = 0;
    virtual void setLightStop() = 0;
    virtual void setLightError() = 0;
    virtual void setLightAlarm() = 0;
    virtual void clearLightAlarm() = 0;

    quint64 m_id;
    bool m_isHandling;

    QList<int> PosList;
    QMap<int, QSharedPointer<RtSlide>> m_posSlideMap;
};

#endif // STAINONLY_BASE_H
