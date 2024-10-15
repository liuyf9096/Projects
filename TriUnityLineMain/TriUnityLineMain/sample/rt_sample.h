#ifndef RT_SAMPLE_H
#define RT_SAMPLE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonValue>
#include <QSharedPointer>
#include <QMap>

enum class SampleStatus {
    Emergency,
    Scaned,
    Send_To_Test,
    SamplingFinished,
    Test_Finished,
    Recycle_To_Rack,
    Review_Sample,
    Need_Smear,
    Send_To_Smear,
    Smear_Done,
    Order_UID,
    Test_UID,
    Canceled
};

class MStation;
class RtSample : public QObject
{
    friend class RtSampleManager;
public:
    explicit RtSample(const QString &sid, QObject *parent = nullptr);
    ~RtSample();

    static QMap<SampleStatus, QString> StatusMap;

    QString sid() { return mSampleId; }

    /* basic information */
    void setRackInfo(const QString &rackid, int pos);

    QString rack_id() { return mRackId; }
    int rack_pos() { return mRackPos; }

    void setBarcode(const QString &barcode);
    QString barcode() { return mBarcode; }

    void setSampleSN(const QString &sn) { mSampleSN = sn; }
    QString sampleSN() { return mSampleSN; }

    /* program list : 5-classify, smear */
    void setProgramList(const QStringList &list);
    QString program();

    void setOrder_uid(const QString &uid);
    QString order_uid() { return m_order_uid; }

    void setTest_uid(const QString &uid);
    QString test_uid() { return m_test_uid; }

    void setTestMode(const QString &mode) { m_testMode = mode; }
    QString testMode() { return m_testMode; }

    void setBloodType(const QString &type) { mBloodType = type; }
    QString bloodtype() { return mBloodType; }

    void setCanceled(bool conceled) { m_isCanceled = conceled; }
    bool isConceled() { return m_isCanceled; }

    void setStation(MStation *station) { mStation = station; }
    MStation *getStation() { return mStation; }

    void setProgramFinished(const QString &program) { m_programList.removeOne(program); }
    bool isProgramFinished() { return m_programList.isEmpty(); }
    bool programContains(const QString &program) { return m_programList.contains(program); }

    bool setOrderInfo(const QJsonObject &obj);
    QJsonObject getOrderInfo() { return m_orderInfo; }
    void setDefaultOrderInfo();
    void addOrderUIDInfo(const QString &key, const QJsonValue &value);

    /* state */
    void setState(SampleStatus status, int value = 1);
    void setState(const QString &status, int value = 1);
    int getState(SampleStatus status);
    int getState(const QString &status);
    bool isProcessDone(SampleStatus status);

private:
    const QString mSampleId;
    QString mRackId;
    int mRackPos;
    QString mBarcode;
    QString mSampleSN;
    QString mBloodType;
    MStation *mStation;
    QJsonObject m_orderInfo;
    QString m_testMode;
    QString m_order_uid;
    QString m_test_uid;

    bool m_isMiniBlood;
    bool m_isEmergency;
    bool m_isCanceled;

    QStringList m_programList;

    QMap<QString, bool> m_statusMap;

    QJsonObject mPatientObj;
};

#endif // RT_SAMPLE_H
