#ifndef RECYCLE_BOX_H
#define RECYCLE_BOX_H

#include "sample/rt_sample_manager.h"

#include <QObject>
#include <QMap>

class RecycleBox : public QObject
{
    Q_OBJECT
public:
    explicit RecycleBox(const QString &box_id, int from, int to, QObject *parent = nullptr);
    ~RecycleBox();

    QString bid() { return m_boxid; }

    void append(int pos, const QString &sid);
    bool isFull();
    bool isEmpty();
    void setMaxCount(int count);

    void eject();
    bool isEjected() { return m_isEjected; }

    void setClearMode();
    bool isClearMode() { return m_isClearMode; }

    int getVacantPos();
    void unloadAllSample();
    void unloadBox();

    void sendBoxToReader();
    void sendBoxToManu();
    void sendReaderSlideInfo();

    int count() { return mSlideMap.count(); }
    bool containsStainSlide();
    bool allStainSlide();

    void setAllSlideDone();

    QJsonArray getBoxInfo();
    QJsonArray getBoxInfo2();

private:
    const QString m_boxid;
    QList<int> mPosList;
    bool m_isClearMode;
    bool m_isEjected;
    int m_maxCount;
    bool m_isUnloadedBox;

    QMap<int, QSharedPointer<RtSlide>> mSlideMap;

    void sendUIRecycleSlideInfo(int pos, const QString &slide_id);
};

#endif // RECYCLE_BOX_H
