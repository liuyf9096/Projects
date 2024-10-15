#ifndef RT_SLIDE_H
#define RT_SLIDE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QSharedPointer>
#include <QMap>

enum class SmearStatus {
    Get_New_Slide,
    Print_Info_Finish,
    WaitF_Add_Blood,
    Add_Sample,
    Smear,
    Wash_Blade,
    Send_To_StainCart,
    Cancel
};

enum class StainStatus {
    Fix,
    A1,
    C1,
    C2,
    Wash,
    Recycle
};

class RtSample;
class RtSlide : public QObject
{
    friend class RtSampleManager;
    Q_OBJECT
public:
    explicit RtSlide(const QString &slide_id, QObject *parent = nullptr);
    ~RtSlide();

    static QMap<SmearStatus, QString> SmearStatusMap;
    static QMap<StainStatus, QString> StainStatusMap;

    QString slide_id() { return mSlideId; }

    void setSlideUID(const QString &uid);
    QString slideUID() { return mSlideUID; }

    void setSampleUID(const QString &uid);
    QString sampleUID() { return mSampleUID; }

    void setSampleID(const QString &id);
    QString sampleID() { return m_sample_id; }

    void setQRcode(const QString &code);
    void setQRcode(const QString &code1, const QString &code2, bool isRet = false);
    QString qrcode() { return m_qrcode; }

    void setSample(QSharedPointer<RtSample> sample) { m_sample = sample; }
    QSharedPointer<RtSample> sample() { return m_sample;}

    void setStainPos(int pos) { mStainPos = pos; }
    int stainPos() { return mStainPos; }

    void setRecycleBoxPos(int pos) { mRecyclePos = pos; }
    int recycleBoxPos() { return mRecyclePos; }

    void setRecycleBoxId(const QString &box_id) { mRecycleBoxId = box_id; }
    QString recycleBoxId() { return mRecycleBoxId; }

    void setRemained();
    bool isRemained() { return m_isRemained; }
    bool isCancelled() { return m_isCancelled; }

    void setSendRequest(bool isSend) { m_isSendRequest = isSend; }
    bool isSendRequest() { return m_isSendRequest; }

    void setStainProcessList(const QStringList &list);
    void addStainProcess(const QString &process);
    QStringList processList() { return m_stainProcessList; }
    void removeStainProcessOne(const QString &step);
    QString getNextStainProcess();

    bool isRet() { return m_isRet; }
    void setRet(bool ret);

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

    void setPrintInfo(const QJsonObject &obj);
    void setDefaultPrintInfo();
    QJsonObject printInfo() { return mPrintInfoObj; }

    void setStatus(StainStatus status, int value);
    int getStatus(StainStatus status);

    void setStatus(SmearStatus status, int value);
    int getStatus(SmearStatus status);

    void sql_record(const QJsonObject &setObj = QJsonObject());
    void sql_recordSmearStatus(SmearStatus status, int value);
    void sql_recordStainStatus(StainStatus status, int value);
    void sql_recordStainStatus(const QString &key, const QString &value);

private:
    const QString mSlideId;
    QString mSlideUID;
    QString mSampleUID;
    QString m_sample_id;
    QString m_qrcode;

    int mStainPos;
    int mRecyclePos;
    QString mRecycleBoxId;
    bool m_isSendRequest;

    QStringList m_stainProcessList;

    QMap<QString, int> m_statusMap;
    QSharedPointer<RtSample> m_sample;

    bool m_isRemained;
    bool m_isCancelled;
    bool m_isRet;

    bool m_isPrintEnable;
    bool m_isSmearEnable;
    bool m_isStainEnable;

    QJsonObject mPrintInfoObj;
};

#endif // RT_SLIDE_H
