#include "stain_import.h"
#include "stain/stain_manager.h"
#include "sql/f_sql_database_manager.h"

StainImport::StainImport(const QString &mid, int pos, QObject *parent)
    : DModuleBase(mid, "import", parent)
    , mPos(pos)
{
    qInfo().noquote() << QString("[%1] :").arg(mUserId) << mPos;
}

bool StainImport::isEmpty()
{
    return m_slide.isNull();
}

bool StainImport::addNewSlide(QSharedPointer<RtSlide> slide)
{
    Q_ASSERT(slide);

    if (m_slide.isNull() && slide) {
        m_slide = slide;
        m_slide->setSendRequest(false);
        m_slide->setStainPos(mPos);

        auto manager = StainManager::GetInstance();
        manager->startCheckTimer(true);

        qDebug().noquote() << QString("Import ADD sid:%1").arg(slide->slide_id());

        if (m_slide->isPrintOnly() || m_slide->isSmearOnly()) {
            manager->needCheckImmediately();
        }

        /* record */
        recordSlotSlideState(mPos, slide);

        return true;
    } else {
        qDebug() << "StainImport add New Slide False. pos:" << slide->slide_id();
    }
    return false;
}

bool StainImport::hasRequest()
{
    if (m_slide && m_slide->isSendRequest() == false) {
        return true;
    } else {
        return false;
    }
}

void StainImport::sendRequest()
{
    if (m_slide->isSendRequest() == false) {
        emit onSendRequest_singal(mUserId, mPos, m_slide->slide_id());
        m_slide->setSendRequest(true);
    }
}

bool StainImport::takeOutSample(const QString &sid, int pos)
{
    if (pos == mPos && m_slide->slide_id() == sid) {
        emit onTakeOutOfPos_signal(pos, sid);

        qDebug() << "[Import] remove slide:" << sid;
        m_slide = nullptr;

        /* record */
        recordModuleSlideState("stain_slide", "doing_slide", nullptr);
        recordSlotSlideState(mPos, nullptr);

        return true;
    }
    return false;
}
