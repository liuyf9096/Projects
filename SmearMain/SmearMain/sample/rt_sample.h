#ifndef RT_SAMPLE_H
#define RT_SAMPLE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QSharedPointer>
#include <QMap>

enum class SampleStatus {
    Scaned,
    Get_Order,
    Sending_To_Test,
    Send_To_Test,
    Mixing,
    Sampling,
    Test_Finished,
    Prepare_Blood,
    Ret_Hatch,
    Recycle_To_Rack,
    Add_Sample,
    Abort
};

class RtSlide;
class RtSample : public QObject
{
    friend class RtSampleManager;
    Q_OBJECT
public:
    explicit RtSample(const QString &sid, QObject *parent = nullptr);
    ~RtSample();

    static QMap<SampleStatus, QString> SampleStatusMap;

    QString sid() { return mSid; }

    /* basic information */
    void setRackInfo(const QString &rackid, int pos);

    QString rack_id() { return mRackId; }
    int rack_pos() { return mRackPos; }

    void setBarcode(const QString &barcode);
    QString barcode() { return mBarcode; }

    void setSampleUID(const QString &uid);
    QString sampleUID() { return mSampleUID; }

    void setSampleID(const QString &id);
    QString sampleID() { return m_sample_id; }

    void setTest_uid(const QString &uid);
    QString test_uid() { return m_test_uid; }

    void setOrder_uid(const QString &uid);
    QString order_uid() { return m_order_uid; }

    void setOrderInfo(const QJsonObject &obj);
    QJsonObject getOrderInfo() { return m_orderInfo; }

    void setBloodType(const QString &type) { mBloodType = type; }
    QString bloodtype() { return mBloodType; }

    void setSmearCount(int count);
    int smearCount() { return m_smearCount; }

    void setHtc(double htc);
    double htc() { return m_hct; }

    bool isMiniBlood() { return m_isMiniBlood; }
    void setMiniBlood(bool mini);

    bool isEmergency() { return m_isEmergency; }
    void setEmergency(bool emergency);

    bool isNeedMix() { return m_isNeedMix; }
    void setIsNeedMix(bool isNeedMix);

    bool isRet() { return m_isRet; }
    void setRet(bool ret);

    bool isCapped() { return m_isCapped; }
    void setCapped(bool capped);

    void setCanceled(bool conceled) { m_isCanceled = conceled; }
    bool isConceled() { return m_isCanceled; }

    void setCoord(int coord) { mCoord = coord + mRackPos; }
    int coord() { return mCoord; }

    bool getSmearParams();

    /* print smear stain */
    void setPrintEnable(bool en);
    void setSmearEnable(bool en);
    void setStainEnable(bool en);

    bool isPrintEnable() { return m_isPrintEnable; }
    bool isSmearEnable() { return m_isSmearEnable; }
    bool isStainEnable() { return m_isStainEnable; }
    bool isPrintOnly() { return m_isPrintEnable && !m_isSmearEnable && !m_isStainEnable; }
    bool isSmearOnly() { return m_isSmearEnable && !m_isStainEnable; }
    bool isStainOnly() { return !m_isSmearEnable && m_isStainEnable; }

    QJsonObject patientObj() { return mPatientObj; }
    void setPatientObj(const QString &key, const QJsonValue &value) { mPatientObj.insert(key, value); }
    void setPatientObj(const QJsonObject &obj) { mPatientObj = obj; }

    void setPrintInfo(const QJsonObject &obj);
    void setDefaultPrintInfo();
    QJsonObject printInfo() { return mPrintInfoObj; }

    void setStainProcessList(const QStringList &list);
    QStringList getStainProcessList() { return m_stainProcessList; }

    void setViscosity(int viscosity);
    int viscosity() { return m_viscosity; }

    int addVolume() { return m_add_volume; }

    void setSmearParams(const QJsonArray &arr);
    QJsonArray smearParams() { return m_smearParams; }

    QList<QSharedPointer<RtSlide>> slides;

    /* state */
    void setStatus(SampleStatus status, int value);
    int getStatus(SampleStatus status);

    void sql_record(const QJsonObject &setObj = QJsonObject());
    void sql_recordSampleStatus(SampleStatus status, int value);

private:
    const QString mSid;

    void _init();

    QString mRackId;
    int mRackPos;
    int mCoord;
    QString mBarcode;
    QString mSampleUID;
    QString m_sample_id;
    QString mBloodType;

    QString m_test_uid;
    QString m_order_uid;
    QJsonObject m_orderInfo;

    bool m_isMiniBlood;
    bool m_isEmergency;
    bool m_isRet;
    bool m_isCapped;
    bool m_isNeedMix;
    bool m_isCanceled;

    bool m_isPrintEnable;
    bool m_isSmearEnable;
    bool m_isStainEnable;

    QJsonObject mPrintInfoObj;

    int m_smearCount;
    int m_smearLevel;

    QStringList m_stainProcessList;

    QMap<QString, int> m_statusMap;

    QJsonObject mPatientObj;

    /* smear params */
    int m_viscosity;
    QJsonArray m_smearParams;

    double m_hct;
    int z_hight;
    int start_pos;
    int expend_time;
    int start_speed;
    int max_speed;
    int length;
    int z_wait;
    int z_start_speed;
    int z_max_speed;
    int z_range;

    int m_add_volume;
};

#endif // RT_SAMPLE_H
