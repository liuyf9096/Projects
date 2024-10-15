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
    void setCanceled(); // waiting overtime, cancel the test and unload the rack
    bool isCanceled() { return m_isCanceled; }
    void setBarcode(const QString &barcode);
    QString barcode() { return m_barcode; }

    void setSmearMode();
    bool isSmearMode() { return m_isSmearMode; }
    void setReviewMode(bool isDone, const QString &mode);

    /* cart */
    void setCartId(const QString &cartid) { m_cartId = cartid; }
    QString cartid() { return m_cartId; }

    /* sample */
    QSharedPointer<RtSample> addNewSample(int pos);
    QSharedPointer<RtSample> sampleAt(int index);

    bool isEmpty() { return mSampleMap.isEmpty(); }
    void sendSamplePath(const QString &node_id);
    void setCoord(int coord);

    /* scan */
    void setSampleExist(int pos, bool exist);
    int getNextScanPos();

    /* abort the rest */
    void abort();   // click stop, stop test and unload rest of the samples

    /* req1: unsend sample to station list */
    QList<QSharedPointer<RtSample>> getUnSendToTestSampleList();
    QList<QSharedPointer<RtSample>> getUnSendToSmearList();

    /* req2: finish test to return back to rack */
    QSharedPointer<RtSample> getUnRecycleSample();
    int getUnTestedPos();

    bool isAllScaned();
    bool isAllSendToTest();
    bool isAllTestFinished();
    bool isAllReviewed();
    bool isAllReviewedDone();
    int minReviewDonePos();

    bool isAllFinished();
    bool isNeedReview();
    void doReview(bool isClassify, bool isSmear);

    void setAllSmearProgram();
    void clearSampleProgram();
    void setTestHalfSmearProgram();

    /* smear */
    bool isSampleSmearMode();

private:
    const QString m_rackId;
    QString m_cartId;
    QString m_barcode;
    quint64 m_index;
    bool m_isCanceled;
    bool m_isAborted;
    int m_coord;
    int m_reviewCount;
    bool m_isSmearMode;

    /* Scan 0~9 */
    enum ScanStatus { UnScaned, Exist, Vacant };
    QMap<int, ScanStatus> m_scanPosMap;

    QMap<int, QSharedPointer<RtSample>> mSampleMap;
};

#endif // RT_RACK_H
