#ifndef RT_SAMPLE_MANAGER_H
#define RT_SAMPLE_MANAGER_H

#include "rt_sample.h"
#include "rt_slide.h"
#include "messagecenter/f_message_center.h"

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QSharedPointer>

enum class SampleProcessState {
    Ready,
    Processing,
    Sampling,
    SamplingFinish,
    StainFinish,
    Canceled,
    Done,
    Fail
};

enum class SlideProcessState {
    Ready,
    Processing,
    ScanCode,
    Sampling,
    SamplingFinish,
    Smearing,
    SmearFinish,
    Staining,
    StainFinish,
    Canceled,
    Done,
    Fail
};

class RtSampleManager : public QObject
{
    Q_OBJECT
public:
    static RtSampleManager *GetInstance();

    bool containSlide(const QString &sid);
    QSharedPointer<RtSample> NewSample();
    QSharedPointer<RtSlide> NewSampleSlide(const QString &sid);
    QSharedPointer<RtSlide> NewSampleSlide();
    QList<QSharedPointer<RtSlide>> NewSampleSlideList(QSharedPointer<RtSample> sample);
    void syncSlide(QSharedPointer<RtSlide> slide, QSharedPointer<RtSample> sample);

    QSharedPointer<RtSample> getSample(const QString &sid);
    QSharedPointer<RtSlide> getSlide(const QString &slide_id);

    void removeSlideOne(const QString &slide_id);
    void removeSlideOne(QSharedPointer<RtSlide> slide);
    void removeSlideAll();
    void removeSampleOne(const QString &sid);
    void removeSampleAll();

    void setAutoTestMode(bool isPrint, bool isSmear, bool isStain);
    bool isAutoPrint() { return m_isAuPrint; }
    bool isAutoSmear() { return m_isAuSmear; }
    bool isAutoStain() { return m_isAuStain; }

    bool allSampleSmearFinished();
    bool hasRemainedSlides();

    void setStainProcessList(const QJsonObject &obj);
    void setStainProcessList(const QStringList &list);
    QStringList StainProcessList() { return mProcessList; }

    void setSlideStainProcess(QSharedPointer<RtSlide> slide);
    void setSlideStainProcess(const QString &sid);

    void setSmearParams(const QJsonObject &params);
    void setSmearParams(const QString &hct_level, bool isCapped, const QJsonObject &params);
    QJsonObject getSmearParams(const QJsonObject &level);
    QJsonObject getSmearParams(bool isCapped, const QString &hct_level);

    /* send UI message */
    void requestSampleOrder(QSharedPointer<RtSample> sample);
    void sendUISampleStatus(const QString &sid, SampleProcessState status);
    void sendUISampleStatus(QSharedPointer<RtSample> sample, SampleProcessState status);
    void sendUISlideStatus(QSharedPointer<RtSlide> slide, SlideProcessState status, int remain = 0);

private:
    explicit RtSampleManager(QObject *parent = nullptr);

    quint64 m_sid;

    /* AutoTest */
    bool m_isAuPrint;
    bool m_isAuSmear;
    bool m_isAuStain;

    QMap<QString, QSharedPointer<RtSample> > m_sampleMap;
    QMap<QString, QSharedPointer<RtSlide> > m_slideMap;

    QStringList mProcessList;

private slots:
    void handleSampleOrder_slot(const JPacket &result, const JPacket &request);
};

#endif // RT_SAMPLE_MANAGER_H
