#ifndef RT_SAMPLE_H
#define RT_SAMPLE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QSharedPointer>
#include <QMap>
#include <QStringList>

enum class SampleStatus {
    Emergency,

    Scaned,

    Send_To_Test,
    SamplingFinished,
    Recycle_To_Rack,

    Test_Finished,

    Review_Sample,
    Review_Done,

    Need_Smear,
    Send_To_Smear,
    Smear_Done,

    Order_UID,
    Test_UID,

    Canceled
};

enum class ReviewMode {
    Unknown,
    Auto_Retest,
    Local_Retest,
    Other_Retest,
    Smear,
    Done
};

class MStation;
class RtSample : public QObject
{
    friend class RtSampleManager;
public:
    explicit RtSample(const QString &sid, QObject *parent = nullptr);
    ~RtSample();

    static QMap<SampleStatus, QString> StatusMap;

    QString sid() { return mSid; }

    /* basic information */
    void setRackInfo(const QString &rackid, int pos);

    QString rack_id() { return mRackId; }
    int rack_pos() { return mRackPos; }

    void setBarcode(const QString &barcode);
    QString barcode() { return mBarcode; }

    void setSampleID(const QString &sample_id);
    QString SampleID() { return m_sample_id; }

    void setCoord(int coord) { mCoord = coord + mRackPos; }
    int coord() { return mCoord; }

    /* program list : 5-classify, smear */
    void setProgramList(const QStringList &list);
    void clearProgramList();
    QStringList programList() { return m_programList; }
    QString nextProgram();
    void setDoingProgram(const QString &program);
    void removeProgram(const QString &program);
    void removeDoingProgram();
    bool programContains(const QString &program) { return m_programList.contains(program); }

    void setOrder_uid(const QString &uid);
    QString order_uid() { return m_order_uid; }

    void setTest_uid(const QString &uid);
    QString test_uid() { return m_test_uid; }
    QStringList test_uid_list() { return m_test_uid_list; }

    void setTestMode(const QString &mode) { m_testMode = mode; }
    QString testMode() { return m_testMode; }

    void setBloodType(const QString &type) { mBloodType = type; }
    QString bloodtype() { return mBloodType; }

    void setCanceled(bool conceled) { m_isCanceled = conceled; }
    bool isConceled() { return m_isCanceled; }

    void setRetestMode(bool isDone, const QString &mode = QString());
    ReviewMode reviewMode() { return m_reviewMode; }
    void doReviewMode(bool isClassify, bool isSmear);
    void setSmearMode();
    bool isSmearMode() { return m_isSmearMode; }

    void setHCT(const QString &hct) { mHct = hct; }
    void setRetestRequirement(const QJsonArray &arr) { m_retestRequirement = arr; }

    void setStation(MStation *station) { mStation = station; }
    MStation *getStation() { return mStation; }

    bool setOrderInfo(const QJsonObject &obj);
    QJsonObject getOrderInfo() { return m_orderInfo; }
    void setDefaultOrderInfo();
    void addOrderUIDInfo(const QString &key, const QJsonValue &value);

    /* state */
    void setState(SampleStatus status, int value);
    void setState(const QString &status, int value);
    int getState(SampleStatus status);
    int getState(const QString &status);
    bool isProcessDone(SampleStatus status);

private:
    const QString mSid;
    QString mRackId;
    int mRackPos;
    int mCoord;
    QString mBarcode;
    QString m_sample_id;
    QString mBloodType;

    MStation *mStation;
    QJsonObject m_orderInfo;
    QString m_testMode;
    QString m_order_uid;
    QString m_test_uid;
    QStringList m_test_uid_list;

    bool m_isMiniBlood;
    bool m_isEmergency;
    bool m_isCanceled;
    bool m_isSmearMode;

    ReviewMode m_reviewMode;
    QString mHct;
    QJsonArray m_retestRequirement;

    QStringList m_programList;
    QString m_doingProgram;

    QMap<QString, bool> m_statusMap;

    QJsonObject mPatientObj;
};

#endif // RT_SAMPLE_H
