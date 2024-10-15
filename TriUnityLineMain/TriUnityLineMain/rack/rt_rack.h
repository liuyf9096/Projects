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
    explicit RtRack(const QString &rackid, quint64 index, QObject *parent = nullptr);
    ~RtRack();

    /* basic info */
    QString rackid() { return m_rackId; }
    int rack_index() { return m_index; }
    void setCanceled(bool isCanceled);
    bool isCanceled() { return m_isCanceled; }

    /* cart */
    void setCartId(const QString &cartid) { m_cartId = cartid; }
    QString cartid() { return m_cartId; }

    /* sample */
    QSharedPointer<RtSample> addNewSample(int pos);
    QSharedPointer<RtSample> sampleAt(int index);
    QSharedPointer<RtSample> lastSample();
    QList<QSharedPointer<RtSample>> samples();
    bool isEmpty();

    /* scan */
    void setSampleExist(int pos, bool exist);
    int getNextScanPos();

    /* req1: unsend sample to station list */
    QList<QSharedPointer<RtSample>> getUnSendToTestList();
    QList<QSharedPointer<RtSample>> getUnSendToSmearList();

    /* req2: finish test to return back to rack */
    QSharedPointer<RtSample> getSamplingFinishedSample();
    int getUnTestedPos();

    bool isAllScaned();
    bool isAllSendToTest();
    bool isAllTestFinished();
    bool isAllFinished();
    bool isNeedReview();

    void setAllSampleReviewed();

    void setAllSmearProgram();
    void setTestHalfSmearProgram();

    /* smear */
    bool isSampleSmearMode();

private:
    const QString m_rackId;
    QString m_cartId;
    quint64 m_index;
    bool m_isCanceled;

    /* Scan 0~9 */
    enum ScanStatus { UnScaned, Exist, Vacant };
    QMap<int, ScanStatus> m_scanPosMap;

    QMap<int, QSharedPointer<RtSample>> mSampleMap;
};

#endif // RT_RACK_H
