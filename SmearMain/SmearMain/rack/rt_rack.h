#ifndef RT_RACK_H
#define RT_RACK_H

#include "sample/rt_sample.h"

#include <QObject>
#include <QMap>
#include <QList>
#include <QSharedPointer>

class RtRack : public QObject
{
    Q_OBJECT
public:
    explicit RtRack(const QString &rackid, QObject *parent = nullptr);
    ~RtRack();

    /* basic info */
    QString rackid() { return m_rackId; }
    int rack_index() { return m_index; }
    void setCanceled(); // waiting overtime, cancel the test and unload the rack
    bool isCanceled() { return m_isCanceled; }
    QString barcode() { return m_barcode; }

    void setCartId(const QString &cartid) { m_cartId = cartid; }
    QString cartid() { return m_cartId; }

    /* sample */
    QSharedPointer<RtSample> addNewSample(int pos);
    QSharedPointer<RtSample> sampleAt(int index);

    bool isEmpty() { return mSampleMap.isEmpty(); }
    void unloadAllSample();
    void setCoord(int coord);

    /* scan */
    void setSampleExist(int pos, bool exist);
    int getNextScanPos();

    /* abort the rest */
    void abort(); // click stop, stop test and unload the rack

    /* req1: unsend sample to station list */
    QList<QSharedPointer<RtSample>> getUnSendToTestList();

    /* req2: finish test to return back to rack */
    QSharedPointer<RtSample> getUnRecycleSample();

    int getUnFinishedPos();

    bool isAllScaned();
    bool isAllRecycleFinished();
    bool isAllTestFinished();

private:
    const QString m_rackId;
    QString m_cartId;
    QString m_barcode;
    quint64 m_index;
    bool m_isCanceled;
    bool m_isAborted;
    int m_coord;

    /* Scan 0~9 */
    enum PoseStatus { UnScaned, Exist, Vacant };
    QMap<int, PoseStatus> m_scanPosMap;

    QMap<int, QSharedPointer<RtSample>> mSampleMap;
};

#endif // RT_RACK_H
