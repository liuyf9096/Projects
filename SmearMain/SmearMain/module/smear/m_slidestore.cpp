#include "m_slidestore.h"
#include "m_smear.h"
#include "f_common.h"

#include <QDate>
#include <QTime>
#include <QTimer>

MSlideStore::MSlideStore(const QString &mid, QObject *parent)
    : DModuleBase(mid, "slidestore", parent)
{
    m_printMode = 2;
    m_isPrintEn = FCommon::GetInstance()->isPrintEnable();
    m_isScanSlideCode = FCommon::GetInstance()->getConfigValue("sampling", "scan_slide_en").toBool();

    mTimer = new QTimer(this);
    mTimer->setInterval(100);
    connect(mTimer, &QTimer::timeout, this, &MSlideStore::onStateTimer_slot);

    state_init();

    auto smear = RtDeviceManager::GetInstance()->smear();
    connect(smear, &DSample::onFunctionFinished_signal,
            this, &MSlideStore::onFunctionFinished_slot);
}

void MSlideStore::state_init()
{
    m_isNoSlideStore = false;
    m_slide = nullptr;

    mStoreIndex = Store_None;
    s_state = SlideState::Idle;
}

void MSlideStore::addNewSlideRequest(QSharedPointer<RtSlide> slide)
{
    Q_ASSERT(slide);

    mRequestList.append(slide);
    qDebug() << "[SlideStore] add request:" << slide->slide_id();
}

void MSlideStore::addPrintOnlyRequest(quint64 id, const QJsonArray &arr)
{
    for (int i = 0; i < arr.count(); ++i) {
        QJsonObject obj = arr.at(i).toObject();
        QString sample_uid = obj.value("sample_uid").toString();
        QString slide_uid = obj.value("slide_uid").toString();
        QString sample_id = obj.value("sample_id").toString();
        QJsonObject printObj = obj.value("print").toObject();

        auto slide = RtSampleManager::GetInstance()->NewSampleSlide();
        slide->setSampleUID(sample_uid);
        slide->setSlideUID(slide_uid);
        slide->setSampleID(sample_id);
        slide->setQRcode(sample_id);
        slide->setPrintEnable(true);
        slide->setSmearEnable(false);
        slide->setStainEnable(false);
        slide->setPrintInfo(printObj);

        mPrintOnlyList.append(slide);
    }
    qDebug() << "print only. id:" << id;
}

void MSlideStore::cancelPrintOnlyRequest()
{
    foreach (auto slide, mPrintOnlyList) {
        slide->setStatus(SmearStatus::Cancel, 1);
        RtSampleManager::GetInstance()->sendUISlideStatus(slide, SlideProcessState::Canceled);
        RtSampleManager::GetInstance()->removeSlideOne(slide->slide_id());
    }
    mPrintOnlyList.clear();
}

void MSlideStore::setPrintMode(const QJsonObject &obj)
{
    QString code = obj.value("code").toString();
    if (code == "barcode") {
        m_printMode = 1;
    } else if (code == "QRcode") {
        m_printMode = 2;
    } else if (code == "text") {
        m_printMode = 0;
    } else {
        m_printMode = 2;
    }
    dev->print()->cmd_SetPrinterMode(m_printMode);

    QJsonObject printObj = obj.value("print").toObject();
    QString line1 = printObj.value("line1").toString();
    QString line2 = printObj.value("line2").toString();
    QString line3 = printObj.value("line3").toString();

    qInfo() << "Set Print Info:" << code << line1 << line2 << line3;
}

void MSlideStore::start()
{
    mTimer->start();
}

void MSlideStore::reset()
{
    state_init();
}

void MSlideStore::stop()
{
    mTimer->stop();
}

bool MSlideStore::cmd_NewSlide(StoreIndex index, bool print)
{
    if (index == Store_1) {
        if (print == true) {
            m_api = "NewSlide1_ToPrintPos";
            return dev->smear()->cmd_NewSlide1_ToPrintPos();
        } else {
            m_api = "NewSlide1_ToAddSamplingPos";
            return dev->smear()->cmd_NewSlide1_ToAddSamplingPos();
        }
    } else if (index == Store_2) {
        if (print == true) {
            m_api = "NewSlide2_ToPrintPos";
            return dev->smear()->cmd_NewSlide2_ToPrintPos();
        } else {
            m_api = "NewSlide2_ToAddSamplingPos";
            return dev->smear()->cmd_NewSlide2_ToAddSamplingPos();
        }
    } else {
        qDebug() << "NewSlide error:" << index;
    }
    return false;
}

void MSlideStore::onStateTimer_slot()
{
    switch (s_state) {
    case SlideState::Idle:
        if (mSmear->isCartLoaded() == false) {
            s_state = SlideState::Get_Request;
        }
        break;

    case SlideState::Get_Request:
        if (m_slide != nullptr) {
            logProcess("SlideStore", 1, "New Request", m_slide->slide_id());
            m_slide->setStatus(SmearStatus::Get_New_Slide, 0);
            s_state = SlideState::Check_Slide_Store;
        } else if (mRequestList.isEmpty() == false) {
            m_slide = mRequestList.takeFirst();
        } else if (mPrintOnlyList.isEmpty() == false) {
            m_slide = mPrintOnlyList.takeFirst();
        }
        break;

    case SlideState::Check_Slide_Store:
        dev->smear()->cmd_CheckSensorValue();
        s_state = SlideState::WaitF_Slide_Store;
        break;
    case SlideState::WaitF_Slide_Store:
        if (dev->smear()->isFuncDone("CheckSensorValue")) {
            StoreIndex index = getStoreIndex();
#if 0
            index = Store_1;
#endif
            if (index > Store_None) {
                mStoreIndex = index;
                qDebug() << "newside from store:" << mStoreIndex;

                ExceptionCenter::GetInstance()->removeException("No_Slide_Store_Available", true);

                m_isNoSlideStore = false;
                s_state = SlideState::New_Slide;
            } else {
                if (m_isNoSlideStore == false) {
                    m_isNoSlideStore = true;

                    Exception e("No_Slide_Store_Available", E_Level::Alarm, 101);
                    e.e_msg = "Slide Store Shortage, please renew some slides in any of the box.";
                    ExceptionCenter::GetInstance()->sendExceptionMessage(e);
                }

                s_state = SlideState::Check_Slide_Store;
            }
        } else if (dev->smear()->getFuncResult("CheckSensorValue") > Func_Done) {
            s_state = SlideState::Check_Slide_Store;
        }
        break;

    case SlideState::New_Slide:
    {
        bool ok = cmd_NewSlide(mStoreIndex, m_isPrintEn || m_isScanSlideCode);
        if (ok) {
            //todo clear exception
            QTimer::singleShot(3000, this, [=](){
                dev->sample()->cmd_SlideStoreCleanOpen(mStoreIndex);
            });
            RtSampleManager::GetInstance()->sendUISlideStatus(m_slide, SlideProcessState::Processing);

            s_state = SlideState::WaitF_New_Slide_Done;
        }
    }
        break;
    case SlideState::WaitF_New_Slide_Done:
        if (dev->smear()->getFuncResult(m_api) == Func_Done) {
            dev->sample()->cmd_SlideStoreCleanClose(mStoreIndex);

            mSmear->setSmearCartLoaded(true, m_slide);
            m_slide->setStatus(SmearStatus::Get_New_Slide, 1);

            if (m_isPrintEn == true) {
                if (m_slide->isPrintEnable()) {
                    convertPrintInfo(m_slide);
                    s_state = SlideState::SettingBarcode;
                } else {
                    s_state = SlideState::Distribute_Slide;
                }
            } else if (m_isScanSlideCode == true) {
                s_state = SlideState::Scan_Slide;
            } else if (m_slide->isSmearEnable()) {
                m_slide->setStatus(SmearStatus::WaitF_Add_Blood, 1);
                s_state = SlideState::WaitF_Add_Blood;
            } else {
                s_state = SlideState::Error;
                qDebug() << "Error.";
            }
        } else if (dev->smear()->getFuncResult(m_api) == Func_Fail) {
            auto list = getAvailableIndex();
            if (list.count() > 1) {
                list.removeOne(mStoreIndex);
                mStoreIndex = list.first();
                s_state = SlideState::New_Slide;
            } else {
                s_state = SlideState::Error;
            }
        }
        break;

    case SlideState::SettingBarcode:
        if (m_printMode == 2) {
            m_api = QStringLiteral("PrinterQRcodeSetting");
            QString str = testQRcode();
            if (str.isEmpty() == false) {
                dev->print()->cmd_PrinterQRcodeSetting(str);
            } else {
                dev->print()->cmd_PrinterQRcodeSetting(m_slide->qrcode());
            }
        } else if (m_printMode == 1) {
            m_api = QStringLiteral("PrinterBarcodeSetting");
            dev->print()->cmd_PrinterBarcodeSetting(m_slide->qrcode());
        } else {
            m_api = QStringLiteral("PrinterQRcodeSetting");
            dev->print()->cmd_PrinterQRcodeSetting(m_slide->qrcode());
        }
        s_state = SlideState::SettingRow1;
        break;
    case SlideState::SettingRow1:
        if (dev->print()->isFuncDone(m_api)) {
            dev->print()->cmd_PrinterContentSetting(1, getPrintInfo(m_slide, 1));
            s_state = SlideState::SettingRow2;
        }
        break;
    case SlideState::SettingRow2:
        if (dev->print()->isFuncDone("PrinterContentSetting")) {
            dev->print()->cmd_PrinterContentSetting(2, getPrintInfo(m_slide, 2));
            s_state = SlideState::SettingRow3;
        }
        break;
    case SlideState::SettingRow3:
        if (dev->print()->isFuncDone("PrinterContentSetting")) {
            dev->print()->cmd_PrinterContentSetting(3, getPrintInfo(m_slide, 3));
//            s_state = SlideState::Print_Slide;
            s_state = SlideState::Print_Head_Down;
        }
        break;

        //![Print Slide Info]
    case SlideState::Print_Slide:
        if (dev->print()->isFuncDone("PrinterContentSetting")) {
            bool ok = dev->smear()->cmd_PrintSlideInfo();
            if (ok) {
                logProcess("SlideStore", 2, "Print Info");
                s_state = SlideState::WaitF_Print_Done;
            }
        }
        break;
    case SlideState::WaitF_Print_Done:
        if (dev->smear()->isFuncDone("PrintSlideInfo")) {
            m_slide->setStatus(SmearStatus::Print_Info_Finish, 1);
            s_state = SlideState::Distribute_Slide;
        }
        break;
        //![Print Slide Info]

        //![Print Slide Info]
    case SlideState::Print_Head_Down:
        if (dev->print()->isFuncDone("PrinterContentSetting")) {
            logProcess("SlideStore", 2, "Print Info");
            dev->print()->cmd_PrintHeadDown();
            s_state = SlideState::Print_Test_Info;
        }
        break;
    case SlideState::Print_Test_Info:
        if (dev->print()->isFuncDone("PrintHeadDown")) {
            dev->smear()->cmd_PrintTextData();
            s_state = SlideState::Print_Test_Info1;
        }
        break;
    case SlideState::Print_Test_Info1:
        dev->print()->cmd_PrintTextData();
        s_state = SlideState::Print_QRCode;
        break;
    case SlideState::Print_QRCode:
        if (dev->smear()->isFuncDone("PrintTextData")) {
            dev->smear()->cmd_PrintQRCode();
            s_state = SlideState::Print_QRCode1;
        }
        break;
    case SlideState::Print_QRCode1:
        dev->print()->cmd_PrintQRCode();
        s_state = SlideState::Print_Head_Up;
        break;
    case SlideState::Print_Head_Up:
        if (dev->smear()->isFuncDone("PrintQRCode")) {
            dev->print()->cmd_PrintHeadUp();
            s_state = SlideState::WaitF_Print_Done1;
        }
        break;
    case SlideState::WaitF_Print_Done1:
        if (dev->print()->isFuncDone("PrintHeadUp")) {
            m_slide->setStatus(SmearStatus::Print_Info_Finish, 1);
            s_state = SlideState::Distribute_Slide;
        }
        break;
        //![Print Slide Info]

    case SlideState::Scan_Slide:
        dev->smear()->cmd_SlideScanQRcode_Open(1000);
        m_slideQRCode.clear();
        s_state = SlideState::WaitF_Scan_Done;
        break;
    case SlideState::WaitF_Scan_Done:
        if (dev->smear()->isFuncDone("SlideScanQRcode_Open")) {
            m_slide->setQRcode(m_slideQRCode);
            RtSampleManager::GetInstance()->sendUISlideStatus(m_slide, SlideProcessState::ScanCode);

            s_state = SlideState::Distribute_Slide;
        }
        break;

    case SlideState::Distribute_Slide:
        if (m_slide->isSmearEnable()) {
            s_state = SlideState::Move_To_Add_Sample_Pos;
        } else {
            s_state = SlideState::Move_To_Stain_Cart_Pos;
        }
        break;

    case SlideState::Move_To_Add_Sample_Pos:
    {
        bool ok = dev->smear()->cmd_Slide_FrPrintToAddSamplePos();
        if (ok) {
            logProcess("SlideStore", 3, "Move to AddSample Pos");
            s_state = SlideState::WaitF_Add_Sample_Pos_Done;
        }
    }
        break;
    case SlideState::WaitF_Add_Sample_Pos_Done:
        if (dev->smear()->isFuncDone("Slide_FrPrintToAddSamplePos")) {
            if (m_slide->isSmearEnable()) {
                m_slide->setStatus(SmearStatus::WaitF_Add_Blood, 1);
                s_state = SlideState::WaitF_Add_Blood;
            } else {
                mSmear->addNewSlideRequest(m_slide);
                s_state = SlideState::Finish;
            }
        }
        break;

    case SlideState::WaitF_Add_Blood:
        if (m_slide->getStatus(SmearStatus::Add_Sample) == 1) {
            s_state = SlideState::Finish;
        }
        break;

    case SlideState::Move_To_Stain_Cart_Pos:
    {
        bool ok = dev->smear()->cmd_Slide_FrPrintToStainCartPos();
        if (ok) {
            s_state = SlideState::WaitF_Stain_Cart_Pos_Done;
        }
    }
        break;
    case SlideState::WaitF_Stain_Cart_Pos_Done:
        if (dev->smear()->isFuncDone("Slide_FrPrintToStainCartPos")) {
            mSmear->addNewSlideRequest(m_slide);
            s_state = SlideState::Finish;
        }
        break;

    case SlideState::Finish:
        logProcess("SlideStore", 99, "Finish", m_slide->slide_id());
        m_slide = nullptr;
        s_state = SlideState::Idle;
        break;

    case SlideState::Error:
        break;
    }
}

QString MSlideStore::getPrintInfo(QSharedPointer<RtSlide> slide, int row)
{
    QJsonObject obj = slide->printInfo();
    QString line = QString("line%1").arg(row);
    if (obj.contains(line)) {
        QString content = obj.value(line).toString();
        if (content.length() > 16) {
            content = content.right(16);
            content.replace(0, 2, "..");
        }
        return content;
    } else {
        qWarning() << "can NOT find print info" << obj;
    }
    return QString();
}

void MSlideStore::convertPrintInfo(QSharedPointer<RtSlide> slide)
{
    QJsonObject printObj = slide->printInfo();
    for (int i = 1; i < 4; ++i) {
        QString line = QString("line%1").arg(i);
        if (printObj.contains(line)) {
            QString content = printObj.value(line).toString();
            if (content == "date") {
                content = QDate::currentDate().toString("yyyy-MM-dd");
                printObj.insert(line, content);
            } else if (content == "time") {
                content = QTime::currentTime().toString("HH:mm:ss");
                printObj.insert(line, content);
            }
        }
    }
    slide->setPrintInfo(printObj);

    JPacket p(PacketType::Notification);
    p.module = QStringLiteral("Sample");
    p.api = QStringLiteral("SamplePrintInfo");

    QJsonObject obj;
    obj.insert("sample_uid", slide->sampleUID());
    obj.insert("slide_uid", slide->slideUID());
    obj.insert("print", printObj);

    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);
}

QList<MSlideStore::StoreIndex> MSlideStore::getAvailableIndex()
{
    QList<MSlideStore::StoreIndex> list;
    list.append(Store_1);
    if (dev->smear()->checkSensorValue("SlideStore1_exist") == true) {
        list.append(Store_1);
    }
    if (dev->smear()->checkSensorValue("SlideStore2_exist") == true) {
        list.append(Store_2);
    }
    return list;
}

MSlideStore::StoreIndex MSlideStore::getStoreIndex()
{
    int score1 = 0;
    int score2 = 0;

    if (dev->smear()->checkSensorValue("SlideStore1_exist") == true) {
//        score1 += 10;
        if (dev->smear()->checkSensorValue("SlideStore1_remain") == true) {
            score1 += 10;
        }
    }

    if (dev->smear()->checkSensorValue("SlideStore2_exist") == true) {
//        score2 += 10;
        if (dev->smear()->checkSensorValue("SlideStore2_remain") == true) {
            score2 += 10;
        }
    }

    if (score1 > score2) {
        qDebug().noquote() << QString("choose Slide Store 1, %1:%2").arg(score1).arg(score2);
        return Store_1;
    }
    else if (score1 < score2) {
        qDebug().noquote() << QString("choose Slide Store 2, %1:%2").arg(score1).arg(score2);
        return Store_2;
    }
    else if (score1 > 0) {
        qDebug().noquote() << QString("choose Slide Store 1, %1:%2").arg(score1).arg(score2);
        return Store_1;
    }
    qWarning() << "can NOT choose Slide Store:"<< score1 << ":" << score2;
    return Store_None;
}

QString MSlideStore::testQRcode()
{
    QJsonObject obj = FCommon::getConfigFileValue("print", "self_test").toObject();
    bool enable = obj.value("enable").toBool();
    if (enable) {
        QString str = obj.value("QRcode").toString();
        return str;
    }
    return QString();
}

void MSlideStore::onFunctionFinished_slot(const QString &api, const QJsonValue &resValue)
{
    if (api == "SlideScanQRcode_Open") {
        if (resValue.isObject()) {
            QJsonObject obj = resValue.toObject();
            if (obj.contains("string")) {
                m_slideQRCode = obj.value("string").toString();
            }
        }
    }
}
