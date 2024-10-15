#include "rt_sample_manager.h"
#include "sql/f_sql_database_manager.h"
#include "stain/stain_manager.h"
#include "f_common.h"
#include <QStringList>
#include <QDebug>

RtSampleManager *RtSampleManager::GetInstance()
{
    static RtSampleManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new RtSampleManager();
    }
    return instance;
}

RtSampleManager::RtSampleManager(QObject *parent)
    : QObject(parent)
    , m_sid(1)
{
    qDebug("[RtSampleManager] init..");
    QJsonArray arr = FCommon::GetInstance()->getConfigValue("stain", "stain_process_default").toArray();
    for (QJsonValue v : arr) {
        mProcessList.append(v.toString());
    }

    qInfo() << "Set Default Stain process:" << mProcessList;
    Q_ASSERT(mProcessList.count() > 0);

    m_isAuPrint = true;
    m_isAuSmear = true;
    m_isAuStain = true;

    bool clear = FCommon::GetInstance()->getConfigValue("log", "clear_sample_history").toBool();
    if (clear) {
        m_sid = 1;
    } else {
        int count = FSqlDatabaseManager::GetInstance()->getDatebase("log")->count("samples");
        qDebug() << "Log 'Sample' record count:" << count;
        m_sid = count + 1;
    }

    auto center = FMessageCenter::GetInstance();
    connect(center, &FMessageCenter::onUIMessageResultPacket_signal,
            this, &RtSampleManager::handleSampleOrder_slot);
    qDebug("[RtSampleManager] init..OK.");
    qDebug() << " ";
}

QSharedPointer<RtSample> RtSampleManager::NewSample()
{
    QString sample_id = QString("S%1").arg(m_sid++);
    auto sample = QSharedPointer<RtSample>(new RtSample(sample_id));
    sample->setStainProcessList(mProcessList);

    m_sampleMap.insert(sample_id, sample);

    return sample;
}

QSharedPointer<RtSlide> RtSampleManager::NewSampleSlide(const QString &sid)
{
    auto slide = QSharedPointer<RtSlide>(new RtSlide(sid));

    m_slideMap.insert(sid, slide);
    return slide;
}

QSharedPointer<RtSlide> RtSampleManager::NewSampleSlide()
{
    QString slide_id = QString("S%1-1").arg(m_sid++);
    auto slide = QSharedPointer<RtSlide>(new RtSlide(slide_id));

    m_slideMap.insert(slide_id, slide);
    return slide;
}

/* obsolete */
QList<QSharedPointer<RtSlide> > RtSampleManager::NewSampleSlideList(QSharedPointer<RtSample> sample)
{
    QList<QSharedPointer<RtSlide>> list;
    if (sample) {
        QString sid = sample->sid();
        int count = sample->smearCount();
        if (count > 0 && count <= 10) {
            for (int i = 0; i < count; ++i) {
                QString slide_id = QString("%1-%2").arg(sid).arg(i + 1);
                auto slide = NewSampleSlide(slide_id);
                syncSlide(slide, sample);
                list.append(slide);
            }
        }
    }
    return list;
}

void RtSampleManager::syncSlide(QSharedPointer<RtSlide> slide, QSharedPointer<RtSample> sample)
{
    slide->setSample(sample);

    slide->setSampleUID(sample->sampleUID());
    slide->setSampleID(sample->sampleID());

    if (sample->order_uid().isEmpty() == false) {
        slide->setQRcode(sample->sampleID(), sample->order_uid(), sample->isRet());
    } else {
        slide->setQRcode(sample->sampleID());
    }

    slide->setRet(sample->isRet());

    slide->setPrintEnable(sample->isPrintEnable());
    slide->setSmearEnable(sample->isSmearEnable());
    slide->setStainEnable(sample->isStainEnable());

    if (sample->isStainEnable()) {
        slide->setStainProcessList(mProcessList);
    } else {
        slide->setStainProcessList({"transfer"});
    }
}

QSharedPointer<RtSample> RtSampleManager::getSample(const QString &sid)
{
    if (m_sampleMap.contains(sid)) {
        return m_sampleMap.value(sid);
    } else {
        qWarning() << "can NOT find Sample sid:" << sid;
    }
    return nullptr;
}

bool RtSampleManager::containSlide(const QString &sid)
{
    return m_slideMap.contains(sid);
}

QSharedPointer<RtSlide> RtSampleManager::getSlide(const QString &slide_id)
{
    if (m_slideMap.contains(slide_id)) {
        return m_slideMap.value(slide_id);
    } else {
        qWarning() << "SampleManager can NOT find Slide id:" << slide_id;
    }
    return nullptr;
}

void RtSampleManager::removeSlideOne(const QString &slide_id)
{
    if (m_slideMap.contains(slide_id)) {
        m_slideMap.remove(slide_id);
        qDebug() << "SampleManager remove slide:" << slide_id;
    }
}

void RtSampleManager::removeSlideOne(QSharedPointer<RtSlide> slide)
{
    if (slide) {
        removeSlideOne(slide->slide_id());
    }
}

void RtSampleManager::removeSlideAll()
{
    m_slideMap.clear();
    qDebug() << "delete All sample slides.";
}

void RtSampleManager::removeSampleOne(const QString &sid)
{
    if (m_sampleMap.contains(sid)) {
        m_sampleMap.remove(sid);
        qDebug() << "remove sample, id:" << sid;
    }
}

void RtSampleManager::removeSampleAll()
{
    m_sampleMap.clear();
    qDebug() << "delete All samples.";
}

void RtSampleManager::setAutoTestMode(bool isPrint, bool isSmear, bool isStain)
{
    m_isAuPrint = isPrint;
    m_isAuSmear = isSmear;
    m_isAuStain = isStain;
}

bool RtSampleManager::allSampleSmearFinished()
{
    foreach (auto sample, m_sampleMap) {
        if (sample->isSmearEnable() && sample->getStatus(SampleStatus::Test_Finished) == 0) {
            qDebug() << QString("sample:%1 is not smear done.").arg(sample->sid());
            return false;
        }
    }

    foreach (auto slide, m_slideMap) {
        if (slide->isPrintOnly() && slide->getStatus(SmearStatus::Print_Info_Finish) == 0) {
            qDebug() << QString("slide:%1 is not smear done.").arg(slide->slide_id());
            return false;
        } else if (slide->isSmearEnable() && slide->getStatus(SmearStatus::Smear) == 0) {
            qDebug() << QString("slide:%1 is not smear done.").arg(slide->slide_id());
            return false;
        }
    }
    return true;
}

bool RtSampleManager::hasRemainedSlides()
{
    foreach (auto slide, m_slideMap) {
        if (slide->isRemained()) {
            if (slide->getStatus(StainStatus::Recycle) == 0) {
                return true;
            }
        }
    }
    return false;
}

void RtSampleManager::setStainProcessList(const QJsonObject &obj)
{
    QStringList list;
    if (obj.contains("process")) {
        QJsonArray arr = obj.value("process").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject stepObj = arr.at(i).toObject();
            QString group = stepObj.value("group").toString();

            /* group: "fix", "fixdry", "c1", "c2", "wash" */
            if (SlotsManager::GetInstance()->containGroup(group)) {
               list.append(group);
            }
        }
    }

    mProcessList = list;
    qDebug() << "RtSampleManager set Stain Process:" << mProcessList;
}

void RtSampleManager::setStainProcessList(const QStringList &list)
{
    if (list.isEmpty() || list.contains("transfer") == false) {
        qFatal("set Process Error");
    }

    mProcessList = list;
    qInfo() << "set Stain Process:" << list;
}

void RtSampleManager::setSlideStainProcess(QSharedPointer<RtSlide> slide)
{
    if (slide) {
        if (slide->isStainEnable()) {
            slide->setStainProcessList(mProcessList);
        } else {
            slide->setStainProcessList({"transfer"});
        }
    }
}

void RtSampleManager::setSlideStainProcess(const QString &sid)
{
    if (m_slideMap.contains(sid)) {
        auto slide = m_slideMap.value(sid);
        setSlideStainProcess(slide);
    }
}

void RtSampleManager::setSmearParams(const QJsonObject &params)
{
    if (params.contains("basis")) {

    } else if (params.contains("params")) {
        QString hct_level = params.value("hct_level").toString();
        bool isCapped = params.value("isCapped").toBool();
        QJsonObject smearparams = params.value("params").toObject();
        setSmearParams(hct_level, isCapped, smearparams);
    }
}

void RtSampleManager::setSmearParams(const QString &hct_level, bool isCapped, const QJsonObject &params)
{
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("smearparams");
    if (db) {
        QString table = isCapped ? "capped" : "uncapped";
        QJsonObject whereObj;
        whereObj.insert("hct", hct_level);
        db->updateRecord(table, params, whereObj);
    }
}

QJsonObject RtSampleManager::getSmearParams(const QJsonObject &level)
{
    if (level.contains("isCapped") && level.contains("hct_level")) {
        bool isCapped = level.value("isCapped").toBool();
        QString hct_level = level.value("hct_level").toString();
        return getSmearParams(isCapped, hct_level);
    }
    return QJsonObject();
}

QJsonObject RtSampleManager::getSmearParams(bool isCapped, const QString &hct_level)
{
    auto db = FSqlDatabaseManager::GetInstance()->getDatebase("smearparams");
    if (db) {
        QString query = QString("hct='%1'").arg(hct_level);
        QString table = isCapped ? "capped" : "uncapped";
        QJsonArray arr = db->selectRecord(table, query);
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            return obj;
        } else {
            qWarning() << "can NOT match the corresponding smear parameters." << isCapped << hct_level;
        }
    }
    return QJsonObject();
}

void RtSampleManager::requestSampleOrder(QSharedPointer<RtSample> sample)
{
    bool isUIConnect = FMessageCenter::GetInstance()->isUIConnected();
    if (isUIConnect == false) {
        sample->setSampleUID(sample->sid());

        QString slide_id = QString("%1-1").arg(sample->sid());
        auto slide = NewSampleSlide(slide_id);
        syncSlide(slide, sample);
        sample->slides.append(slide);
        qWarning() << "Default Sample UID:" << sample->sampleUID() << slide->slideUID();
    } else if (sample) {
        JPacket p(PacketType::Request);
        p.module = QStringLiteral("Sample");
        p.api = QStringLiteral("GetSampleOrder");

        QJsonObject obj;
        obj.insert("sid", sample->sid());
        if (sample->barcode().isEmpty() == false) {
            obj.insert("barcode", sample->barcode());
        } else if (sample->sampleID().isEmpty() == false) {
            obj.insert("barcode", sample->sampleID());
        } else if (sample->test_uid().isEmpty() == false) {
            obj.insert("barcode", sample->test_uid());
        }

        obj.insert("rack_id", sample->rack_id());
        obj.insert("rack_pos", sample->rack_pos());
        obj.insert("order", sample->getOrderInfo());
        p.paramsValue = obj;

        FMessageCenter::GetInstance()->sendUIMessage(p);
    }
}

void RtSampleManager::sendUISampleStatus(const QString &sid, SampleProcessState status)
{
    if (m_sampleMap.contains(sid)) {
        auto sample = m_sampleMap.value(sid);
        sendUISampleStatus(sample, status);
    }
}

void RtSampleManager::sendUISampleStatus(QSharedPointer<RtSample> sample, SampleProcessState status)
{
    JPacket p(PacketType::Notification);
    p.module = QStringLiteral("Sample");
    p.api = QStringLiteral("SampleStatus");

    QJsonObject obj;
    obj.insert("sample_uid", sample->sampleUID());
    if (status == SampleProcessState::Ready) {
        obj.insert("status", "ready");
        obj.insert("pos", QString("%1-%2").arg(sample->rack_id()).arg(sample->rack_pos() + 1));
    } else if (status == SampleProcessState::Processing) {
        obj.insert("status", "processing");
    } else if (status == SampleProcessState::Sampling) {
        obj.insert("status", "sampling");
    } else if (status == SampleProcessState::SamplingFinish) {
        obj.insert("status", "samplingFinish");
    } else if (status == SampleProcessState::Canceled) {
        obj.insert("status", "canceled");
    } else if (status == SampleProcessState::Done) {
        obj.insert("status", "done");
    } else if (status == SampleProcessState::Fail) {
        obj.insert("status", "fail");
    } else {
        obj.insert("status", "unkown");
        qWarning() << "sample status unknown.";
    }
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RtSampleManager::sendUISlideStatus(QSharedPointer<RtSlide> slide, SlideProcessState status, int remain)
{
    JPacket p(PacketType::Notification);
    p.module = QStringLiteral("Sample");
    p.api = QStringLiteral("SampleStatus");

    QJsonObject obj;
    obj.insert("sample_uid", slide->sampleUID());
    obj.insert("slide_uid", slide->slideUID());

    if (status == SlideProcessState::Ready) {
        obj.insert("status", "ready");
    } else if (status == SlideProcessState::Processing) {
        obj.insert("status", "processing");
    } else if (status == SlideProcessState::ScanCode) {
        obj.insert("status", "scancode");
        obj.insert("qrcode", slide->qrcode());
    } else if (status == SlideProcessState::Sampling) {
        obj.insert("status", "sampling");
    } else if (status == SlideProcessState::SamplingFinish) {
        obj.insert("status", "samplingFinish");
    } else if (status == SlideProcessState::Smearing) {
        obj.insert("status", "smearing");
    } else if (status == SlideProcessState::SmearFinish) {
        obj.insert("status", "smearFinish");
    } else if (status == SlideProcessState::Staining) {
        obj.insert("status", "staining");
        obj.insert("remain", remain);
    } else if (status == SlideProcessState::StainFinish) {
        obj.insert("status", "stainFinish");
    } else if (status == SlideProcessState::Canceled) {
        obj.insert("status", "canceled");
    } else if (status == SlideProcessState::Done) {
        if (slide->isRemained()) {
            obj.insert("status", "dumped");
        } else {
            obj.insert("status", "done");
            obj.insert("box_pos", slide->recycleBoxPos());
            obj.insert("box_id", slide->recycleBoxId());
        }
    } else if (status == SlideProcessState::Fail) {
        obj.insert("status", "fail");
    } else {
        obj.insert("status", "unkown");
        qWarning() << "slide status unknown.";
    }
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);
}

void RtSampleManager::handleSampleOrder_slot(const JPacket &result, const JPacket &request)
{
    if (request.api == "GetSampleOrder") {
        QJsonObject obj = result.resValue.toObject();

        QString sid = obj.value("sid").toString();

        if (m_sampleMap.contains(sid) == false) {
            qWarning() << "Can NOT find sample, sid:" << sid;
            return;
        }

        auto sample = m_sampleMap.value(sid);

        QString sample_uid = obj.value("sample_uid").toString();
        sample->setSampleUID(sample_uid);

        QString sample_type = obj.value("sample_type").toString();
        if (sample_type.contains("wholeblood", Qt::CaseInsensitive)) {
            sample->setMiniBlood(false);
        } else if (sample_type.contains("miniblood", Qt::CaseInsensitive)) {
            sample->setMiniBlood(true);
        }

        bool emergency = obj.value("emergency").toBool();
        sample->setEmergency(emergency);

        double hct = obj.value("hct").toDouble();
        sample->setHtc(hct);

        QJsonObject modeObj = obj.value("mode").toObject();
        bool isPrint = modeObj.value("isPrint").toBool();
        bool isSmear = modeObj.value("isSmear").toBool();
        bool isStain = modeObj.value("isStain").toBool();
        sample->setPrintEnable(isPrint);
        sample->setSmearEnable(isSmear);
        sample->setStainEnable(isStain);

        QJsonObject smearObj = obj.value("smear").toObject();
        int smearCount = smearObj.value("count").toInt();
        sample->setSmearCount(smearCount);

        QJsonArray arr = obj.value("slides").toArray();
        for (int i = 0; i < arr.count(); ++i) {
            QJsonObject slideObj = arr.at(i).toObject();
            QString slide_uid = slideObj.value("slide_uid").toString();
            QJsonObject printObj = slideObj.value("print").toObject();
            QString sample_id = printObj.value("sample_id").toString();

            QString slide_id = QString("%1-%2").arg(sid).arg(i + 1);

            /* Generate New Slide */
            auto slide = NewSampleSlide(slide_id);
            slide->setSample(sample);
            slide->setSlideUID(slide_uid);
            slide->setSampleUID(sample_uid);
            slide->setSampleID(sample_id);

            if (sample->order_uid().isEmpty() == false) {
                slide->setQRcode(sample->sampleID(), sample->order_uid(), sample->isRet());
            } else {
                slide->setQRcode(sample->sampleID());
            }

            slide->setPrintEnable(isPrint);
            slide->setSmearEnable(isSmear);
            slide->setStainEnable(isStain);

            slide->setPrintInfo(printObj);
            setSlideStainProcess(slide);

            sample->setSampleID(sample_id);
            sample->slides.append(slide);
        }

        sample->setStatus(SampleStatus::Get_Order, 1);
        sendUISampleStatus(sample, SampleProcessState::Ready);
    }
}
