#include "exception_center.h"
#include "messagecenter/f_message_center.h"
#include "process/process_manager.h"
#include "device/rt_device_base.h"
#include "track/track_manager.h"
#include "stain/slot/slots_manager.h"
#include "stain/recyclebox/m_recyclebox_mgr.h"
#include "process/m_remove_remain_tube.h"

static const QStringList ItemTypeList = {"can", "serial", "memory", "function", "motor", "optocoupler"};

Exception::Exception(const QString &error_id, E_Level level, int code)
    : e_id(error_id)
    , e_code(code)
    , e_level(level)
    , e_module("Exception")
{
    e_type = Exception_Type::Unknown;
}

ExceptionCenter *ExceptionCenter::GetInstance()
{
    static ExceptionCenter *instance = nullptr;
    if (instance == nullptr) {
        instance = new ExceptionCenter();
    }
    return instance;
}

QString ExceptionCenter::getItemTypeS(int itemType)
{
    if (itemType > 0 && itemType < ItemTypeList.count()) {
        return ItemTypeList.at(itemType);
    }
    return QString();
}

QString ExceptionCenter::getErrorTypeS(quint8 itemType, quint8 errorType)
{
    if (itemType == 6) {    // optocoupler
        switch (errorType) {
        case 1:
            return "Cover-Error";
            break;
        case 2:
            return "UnCover-Error";
            break;
        case 3:
            return "Unknow-Coverage";
            break;
        default:
            break;
        }
    } else if (itemType == 5) { // motor
        switch (errorType) {
        case 1:
            return "Reset-Error";
            break;
        case 2:
            return "PreReset-Error";
            break;
            /* ... */
        default:
            break;
        }
    }
    return QString();
}

ExceptionCenter::ExceptionCenter(QObject *parent)
    : QObject{parent}
{

}

void ExceptionCenter::addException(const QString &error_id, Exception exp)
{
    m_ExpMap.insert(error_id, exp);
}

void ExceptionCenter::removeException(const QString &error_id, bool isSendUI)
{
    if (m_ExpMap.contains(error_id)) {
        m_ExpMap.remove(error_id);

        if (isSendUI) {
            sendClearExceptionMessage(error_id);
        }
    }
}

void ExceptionCenter::sendExceptionMessage(const Exception &exp)
{
    JPacket p(PacketType::Notification);
    p.module = exp.e_module;
    p.api = "SetException";

    QJsonObject obj;
    obj.insert("source", "MidCtrl");
    obj.insert("code", exp.e_code);
    obj.insert("error_id", exp.e_id);
    obj.insert("message", exp.e_msg);

    if (exp.e_level == E_Level::Alarm) {
        obj.insert("level", "Alarm");
    } else if (exp.e_level == E_Level::Error) {
        obj.insert("level", "Error");
    } else {
        obj.insert("level", "Unkown");
    }

    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);

    m_ExpMap.insert(exp.e_id, exp);
}

#if 0
void ExceptionCenter::sendExceptionObj(const QString &error_id, const QJsonObject &obj)
{
    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);

    Exception exp(error_id);
    m_ExpMap.insert(error_id, exp);
}
#endif

void ExceptionCenter::sendClearExceptionMessage(const QString &error_id)
{
    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "ClearException";

    QJsonObject obj;
    obj.insert("error_id", error_id);
    p.paramsValue = obj;

    FMessageCenter::GetInstance()->sendUIMessage(p);

    m_ExpMap.remove(error_id);
}

void ExceptionCenter::handleClearException(const QString &error_id, quint64 id)
{
    if (error_id == "MixRemainTube_101") {
        MProcessManager::GetInstance()->mRemoveRemianTube->startProcess(MRemoveRemainTube::InMix, id);
    }
    else if (error_id == "SamplingCartRemainTube_101") {
        MProcessManager::GetInstance()->mRemoveRemianTube->startProcess(MRemoveRemainTube::InCart, id);
    }
    else if (error_id == "SmearCartRemainSlide_101") {
        JPacket p(PacketType::Result, id);
        p.errorMessage = true;
        FMessageCenter::GetInstance()->sendUIMessage(p);
    }
    else if (error_id == "StainSlideRemain_101") {
        MProcessManager::GetInstance()->startProcess("removeRemainStainSlides", id);
    }
    else if (error_id == "StainSlot_Not_Clean") {
        SlotsManager::GetInstance()->cleanRemainSlots(id);
    }
    else if (error_id == "LoadNewRack_101") {
        TrackManager::GetInstance()->mImport->clearImportError();
        ExceptionCenter::GetInstance()->removeException("LoadNewRack_101", false);
        FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, id);
    }
    else if (error_id == "stain_NewRecycleBox_80") {
        MRecycleBoxMgr::GetInstance()->renewBox();
        ExceptionCenter::GetInstance()->removeException("stain_NewRecycleBox_80", false);
        FMessageCenter::GetInstance()->sendDoneResultMessage(Commun_UI, id);
    }
    else if (error_id == "Boot_80") {
        MProcessManager::GetInstance()->startProcess("boot", id);
    }
    else if (error_id == "cart1_Reset") {
        TrackManager::GetInstance()->mCarts->mCart1->reset();
        TrackManager::GetInstance()->mCarts->mCart1->cmd_Reset();
    }
    else if (error_id == "cart2_Reset") {
        TrackManager::GetInstance()->mCarts->mCart2->reset();
        TrackManager::GetInstance()->mCarts->mCart2->cmd_Reset();
    }
    else if (error_id.startsWith("Lack_of") || error_id.endsWith("_Full")) {
        JPacket p(PacketType::Result, id);
        p.resValue = true;
        FMessageCenter::GetInstance()->sendUIMessage(p);
    }
    else {
        JPacket p(PacketType::Error, id);
        p.errorMessage = "can NOT find Exception called:" + error_id;
        FMessageCenter::GetInstance()->sendUIMessage(p);
    }
}

bool ExceptionCenter::isAlarmException()
{
    for (const auto &exp : qAsConst(m_ExpMap)) {
        if (exp.e_level == E_Level::Alarm) {
            return true;
        }
    }
    return false;
}

bool ExceptionCenter::isErrorException()
{
    for (const auto &exp : qAsConst(m_ExpMap)) {
        if (exp.e_level == E_Level::Error) {
            return true;
        }
    }
    return false;
}

void ExceptionCenter::onFunctionFinished_slot(const QString &api, const QJsonValue &)
{
    Q_UNUSED(api)
}

void ExceptionCenter::onFunctionFailed_slot(const QString &api, const QJsonObject &errorObj)
{
    auto device = qobject_cast<RtDeviceBase*>(sender());
    Q_ASSERT(device);

    if (api == "Import_Load") {
        return;
    }

    //ERROR[3]
    int function_id = errorObj.value("function_id").toInt();
    QString error_id = QString("%1%2").arg(device->boardNum(), 2, 10, QChar('0')).arg(function_id, 3, 10, QChar('0'));
    QString error_code = QString("%1%2").arg(error_id).arg(0, 2, 10, QChar('0'));

    QJsonObject obj = errorObj;
    obj.insert("source", "device");
    obj.insert("code", 80);
    obj.insert("error_id", error_id);
    obj.insert("error_code", error_code);
    obj.insert("message", QString("Command %1 Fail. Module:%2").arg(api).arg(device->deviceID()));

    Exception exp(error_id);
    exp.e_type = Exception_Type::Action_Fail;
    exp.e_level = E_Level::Error;
    exp.e_code = 80;

    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;
    FMessageCenter::GetInstance()->sendUIMessage(p);

    m_ExpMap.insert(error_id, exp);
}

void ExceptionCenter::onFunctionTimeout_slot(const QString &api, const QJsonObject &infoObj)
{
    auto device = qobject_cast<RtDeviceBase*>(sender());
    Q_ASSERT(device);

    QJsonObject obj = infoObj;
    int function_id = infoObj.value("function_id").toInt();
    QString error_id = QString("%1%2").arg(device->boardNum(), 2, 10, QChar('0')).arg(function_id, 3, 10, QChar('0'));
    QString error_code = QString("%1%2").arg(error_id).arg(99, 2, 10, QChar('0'));

    obj.insert("device_id", device->deviceID());
    obj.insert("source", "midCtrl");
    obj.insert("code", 99);
    obj.insert("error_id", error_id);
    obj.insert("error_code", error_code);
    obj.insert("message", QString("Command %1 Timeout. Module:%2").arg(api).arg(device->deviceID()));
    obj.insert("api", api);

    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;

    Exception exp(error_id, E_Level::Error, 99);
    exp.e_type = Exception_Type::Action_Timeout;

    FMessageCenter::GetInstance()->sendUIMessage(p);

    m_ExpMap.insert(error_id, exp);
}
