#include "f_message_center.h"
#include "servers/websocket/f_websocket_manager.h"
#include "servers/websocket/f_websocket_server.h"
#include "module/module_manager.h"
#include "device/rt_device_manager.h"
#include "process/process_manager.h"
#include "track/track_manager.h"
#include "stain/stain_manager.h"
#include "smear/smear_manager.h"
#include "unity/m_unity_task.h"
#include "light/m_lights.h"
#include "f_common.h"
#include <QMetaType>

FMessageCenter *FMessageCenter::GetInstance()
{
    static FMessageCenter *instance = nullptr;
    if (instance == nullptr) {
        instance = new FMessageCenter();
    }
    return instance;
}

FMessageCenter::FMessageCenter(QObject *parent)
    : QObject(parent)
    , m_canbus_id(0)
    , m_ws_id(0)
{
    qRegisterMetaType<JPacket>("JPacket");

    /* socket To canbus */
    mCanbus = FWebSocketManager::GetInstance()->getSocket("Canbus");
    connect(mCanbus, &FWebSocket::onReceiveMessageObj_signal,
            this, &FMessageCenter::onReceiveCanbusMessage_slot);

    /* socket To unity */
    mUnity = FWebSocketManager::GetInstance()->getSocket("Unity");
    if (FCommon::GetInstance()->isUnited() == true) {
        connect(mUnity, &FWebSocket::onReceiveMessageObj_signal,
                this, &FMessageCenter::onReceiveUnityMessage_slot);
        connect(mUnity, &FWebSocket::onNewServerAddress_signal,
                this, &FMessageCenter::onNewUnityServerAddress_slot);
    }

    /* socket To reader */
    mReader = FWebSocketManager::GetInstance()->getSocket("Reader");
    if (FCommon::GetInstance()->isReaderEnable()) {
        connect(mReader, &FWebSocket::onConnected_signal, this, [=](){
            sendReaderConnectionStatus(true);
        });
        connect(mReader, &FWebSocket::onDisconnected_signal, this, [=](){
            sendReaderConnectionStatus(false);
        });
        connect(mReader, &FWebSocket::onReceiveMessageObj_signal,
                this, &FMessageCenter::onReceiveReaderMessage_slot);
        connect(mReader, &FWebSocket::onNewServerAddress_signal,
                this, &FMessageCenter::onNewReaderServerAddress_slot);
    }

    /* websocket server To ui */
    auto wsServer = FWebSocketServer::GetInstance();
    wsServer->setReceivingExemptWords(QStringList() << "SensorValue");

    connect(wsServer, &FWebSocketServer::onClientConnected_signal, this, [=](const QString &client_id){
        mUI_clientid = client_id;
        qInfo() << "UI connected." << client_id;

        if (FCommon::GetInstance()->isReaderEnable()) {
            sendReaderConnectionStatus();
        }
    });
    connect(wsServer, &FWebSocketServer::onClientDisconnected_signal, this, [=](const QString &client_id){
        if (client_id == mUI_clientid) {
            mUI_clientid.clear();
        }
    });
    connect(wsServer, &FWebSocketServer::onReceiveMessageJsonObj_signal,
            this, &FMessageCenter::onReceiveUIMessage_slot);
}

bool FMessageCenter::isUIConnected()
{
    return !mUI_clientid.isEmpty();
}

bool FMessageCenter::isUnityConnected()
{
    return mUnity->isConnected();
}

bool FMessageCenter::isCanbusConnected()
{
    return mCanbus->isConnected();
}

bool FMessageCenter::isReaderConnected()
{
    return mReader->isConnected();
}

quint64 FMessageCenter::sendCanbusMessage(JPacket &request, const QString &username)
{
    Q_UNUSED(username)

    quint64 id = m_canbus_id++;

    request.id = id;
    if (request.device.isEmpty()) {
        request.device = QStringLiteral("CanbusServer");
    }

    m_CanbusPacketMap.insert(request.id, request);
    mCanbus->sendMessage(request);
    return id;
}

quint64 FMessageCenter::sendUIMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("UI");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }

    FWebSocketServer::GetInstance()->sendPacket(mUI_clientid, packet);
    return packet.id;
}

quint64 FMessageCenter::sendUnityMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("Unity");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }

    mUnity->sendMessage(packet);
    return packet.id;
}

quint64 FMessageCenter::sendReaderMessage(JPacket &packet)
{
    if (packet.device.isEmpty()) {
        packet.device = QStringLiteral("Reader");
    }
    if (packet.type == PacketType::Request) {
        packet.id = m_ws_id++;
        m_WsPacketMap.insert(packet.id, packet);
    }

    mReader->sendMessage(packet);
    return packet.id;
}

void FMessageCenter::sendDoneResultMessage(Commun_Destination des, quint64 id)
{
    JPacket p(PacketType::Result, id);
    p.resValue = true;

    if (des == Commun_UI) {
        sendUIMessage(p);
    } else if (des == Commun_United) {
        sendUnityMessage(p);
    } else if (des == Commun_Reader) {
        sendReaderMessage(p);
    }
}

void FMessageCenter::onReceiveUIMessage_slot(const QString &client_id, const QJsonObject &msgObj)
{
    if (client_id != mUI_clientid) {
        qDebug() << "client id is Unknown." << client_id;
        return;
    }

    JPacket packet = FJsonRpcParser::decode(msgObj);
    PacketType type = PacketType::None;

    if (packet.type == PacketType::Request) {
        if (packet.module == "Process") {
            MProcessManager::GetInstance()->handleProcess(packet);
            return;
        } else if (packet.module == "Config") {
            configModule(packet);
            return;
        } else if (packet.module == "Emergency") {
            TrackManager::GetInstance()->mEmergency->handleRequest(packet);
            MRecycleBoxMgr::GetInstance()->startRun(true);
            return;
        } else if (packet.module == "Recycle") {
            if (packet.api == "RenewRecycleBox") {
                MRecycleBoxMgr::GetInstance()->renewBox();
                type = PacketType::Result;
            }
        } else if (packet.module == "AutoTest") {
            if (packet.api == "Start") {
                DModuleManager::GetInstance()->startAutoTest();
                MRecycleBoxMgr::GetInstance()->startRun(true);
                type = PacketType::Result;
            } else if (packet.api == "Suspend") {
                DModuleManager::GetInstance()->suspendAutoTest();
                type = PacketType::Result;
            } else if (packet.api == "Stop") {
                DModuleManager::GetInstance()->stopAutoTest();
                type = PacketType::Result;
            } else if (packet.api == "Abort") {
                DModuleManager::GetInstance()->stopAutoTest();
                type = PacketType::Result;
            }
        } else if (packet.module == "StainOnly") {
            if (packet.api == "Start") {
                QJsonObject obj = packet.paramsValue.toObject();
                StainManager::GetInstance()->startStainOnlyProcess(packet.id, obj);
                MRecycleBoxMgr::GetInstance()->startRun(true);
                return;
            } else if (packet.api == "Stop") {
                StainManager::GetInstance()->cancelStainOnlyRequest();
                type = PacketType::Result;
            }
        } else if (packet.module == "PrintOnly") {
            if (packet.api == "Start") {
                QJsonObject obj = packet.paramsValue.toObject();
                QJsonArray arr = obj.value("slides").toArray();
                SmearManager::GetInstance()->mSlideStore->addPrintOnlyRequest(packet.id, arr);
                MRecycleBoxMgr::GetInstance()->startRun(true);
                type = PacketType::Result;
            } else if (packet.api == "Stop") {
                SmearManager::GetInstance()->mSlideStore->cancelPrintOnlyRequest();
                type = PacketType::Result;
            }
        } else if (packet.module == "RecycleBox") {
            if (packet.api == "Eject") {
                bool ok = MRecycleBoxMgr::GetInstance()->ejectCurrentBox();
                type = ok ? PacketType::Result : PacketType::Error;
            }
        } else if (packet.module == "Maintain") {
            if (packet.api == "FillPools") {
                QJsonObject obj = packet.paramsValue.toObject();
                SlotsManager::GetInstance()->startFillPool(packet.id, obj);
                type = PacketType::Result;
            } else if (packet.api == "DrainPools") {
                QJsonObject obj = packet.paramsValue.toObject();
                SlotsManager::GetInstance()->startFillPool(packet.id, obj);
                type = PacketType::Result;
            }
        } else if (packet.module == "Slot") {
            SlotsManager::GetInstance()->handleSlotRequest(packet);
            return;
        } else if (packet.module == "MainLight") {
            MLights::GetInstance()->handleMainlight(packet.api, packet.paramsValue.toObject());
            type = PacketType::Result;
        } else if (packet.module == "Solution") {
            if (packet.api == "Maintain") {
                QJsonObject obj = packet.paramsValue.toObject();
                QString method = obj.value("method").toString();
                QJsonArray arr = obj.value("groups").toArray();
                SlotsManager::GetInstance()->setStainMethod(method);
                SlotsManager::GetInstance()->setSolutionExpiration(arr);
                type = PacketType::Result;
            }
        } else if (packet.module == "Version") {
            if (packet.api == "GetVersion") {
                JPacket p(PacketType::Result, packet.id);
                QJsonObject obj;
                obj.insert("version", FCommon::appVersion());
                obj.insert("releasedate", FCommon::releaseDate());
                p.resValue = obj;
                sendUIMessage(p);
                return;
            }
        } else if (packet.module == "Exception") {
            if (packet.api == "ClearException") {
                auto obj = packet.paramsValue.toObject();
                QString error_id = obj.value("error_id").toString();

                ExceptionCenter::GetInstance()->handleClearException(error_id, packet.id);
                return;
            }
        }

        if (type != PacketType::None) {
            JPacket p(type, packet.id);
            sendUIMessage(p);
        } else {
            qDebug() << "can NOT handle UI api." << packet;
        }
    } else if (packet.type == PacketType::Result) {
        if (m_WsPacketMap.contains(packet.id)) {
            auto request = m_WsPacketMap.take(packet.id);
            emit onUIMessageResultPacket_signal(packet, request);
        }
    }
}

void FMessageCenter::onReceiveCanbusMessage_slot(const QJsonObject &obj)
{
    JPacket packet = FJsonRpcParser::decode(obj);
    if (packet.type == PacketType::Notification) {
        if (packet.api == "Notification") {
            handleKeyNotification(packet);
        } else if (packet.api == "Exception") {
            handleCanbusException(packet);
        } else if (packet.api == "GripperStatus") {
            emit gripperStatusInfo_signal(packet.paramsValue.toObject());
        } else {
            emit onReceiveCanbusNotification_signal(packet);
        }
    } else if (m_CanbusPacketMap.contains(packet.id)) {
        auto request = m_CanbusPacketMap.take(packet.id);
        emit onReceiveCanbusPacket_signal(packet, request);
    } else {
        qDebug() << "can NOT handle Canbus obj:" << obj;
    }
}

bool FMessageCenter::handleKeyNotification(JPacket &packet)
{
    if (packet.paramsValue.isObject()) {
        QJsonObject paramsObj = packet.paramsValue.toObject();
        int cmd = paramsObj.value("cmd").toInt();
        int arg = paramsObj.value("arg").toInt();
        if (cmd == 0xE041) {
            if (arg == 1) {
                JPacket p(PacketType::Notification);
                p.module = "Notification";
                p.api = "KeyClicked";
                p.paramsValue = "emergency";
                sendUIMessage(p);
            } else if (arg == 2) {
                JPacket p(PacketType::Notification);
                p.module = "Notification";
                p.api = "KeyClicked";
                p.paramsValue = "mode";
                sendUIMessage(p);
            }
            return true;
        }
    }
    return false;
}

void FMessageCenter::handleCanbusException(JPacket &packet)
{
    QJsonObject obj = packet.paramsValue.toObject();

#if 0
    int board_id = obj.value("board_id").toInt();
    int cmd = obj.value("cmd").toInt();
    int item_id = obj.value("item_id").toInt();
#endif

    int function_id = obj.value("function_id").toInt();
    int index_id = obj.value("index_id").toInt();
    int item_type_id = obj.value("item_type").toInt();
    QString item_type = ExceptionCenter::getItemTypeS(item_type_id - 1);

    int error_type_id = obj.value("error_type").toInt();
    QString error_type = ExceptionCenter::getErrorTypeS(item_type_id, error_type_id);

    obj.insert("item_type", item_type);
    obj.insert("item_type_id", item_type_id);
    obj.insert("error_type", error_type);
    obj.insert("error_type_id", error_type_id);
    obj.insert("source", "device");

    int board_num = RtDeviceManager::GetInstance()->getBoardNum(packet.module);
    QString error_id = QString("%1%2").arg(board_num, 2, 10, QChar('0')).arg(function_id, 3, 10, QChar('0'));
    QString error_code = QString("%1%2").arg(error_id).arg(index_id, 2, 10, QChar('0'));
    obj.insert("error_id", error_id);
    obj.insert("error_code", error_code);

    JPacket p(PacketType::Notification);
    p.module = "Exception";
    p.api = "SetException";
    p.paramsValue = obj;
    sendUIMessage(p);
}

void FMessageCenter::onReceiveUnityMessage_slot(const QJsonObject &obj)
{
//    qDebug() << "[Unity] >>" << obj;

    JPacket packet = FJsonRpcParser::decode(obj);
    if (packet.type == PacketType::Request) {
        auto task = DModuleManager::GetInstance()->mUnityTask;
        if (packet.api == "IsReceiveNewSampleAble") {
            task->checkAvailable(packet.id, packet.paramsValue.toObject());
        } else if (packet.api == "TakeUpNewSample") {
            MRecycleBoxMgr::GetInstance()->startRun(true);
            task->takeUpNewSample(packet.id, packet.paramsValue.toObject());
        } else if (packet.api == "RecycleSampleToRack") {
            task->recycleSample(packet.id, packet.paramsValue.toObject());
        } else if (packet.api == "RotateTube") {
            task->rotateTube(packet.id);
        }
    } else if (packet.type == PacketType::Result) {
        auto request = m_WsPacketMap.take(packet.id);
        emit onUnityMessageResult_signal(packet, request);
    } else {
        qDebug() << "Unknown Unity Message:" << packet;
    }
}

void FMessageCenter::onNewUnityServerAddress_slot(const QString &address, quint16 port)
{
    if (mUnity->address() != address || mUnity->port() != port) {
        mUnity->setServerAddressPort(address, port);

        FCommon::setConfigFileValue("united", "websocket", "address", address);
        FCommon::setConfigFileValue("united", "websocket", "port", port);
    }
}

void FMessageCenter::sendReaderConnectionStatus(bool on)
{
    JPacket p(PacketType::Notification);
    p.module = "Reader";
    p.api = "ConnectionStatus";

    QJsonObject obj;
    obj.insert("isConnected", on);
    obj.insert("address", mReader->address());
    obj.insert("port", mReader->port());
    p.paramsValue = obj;

    sendUIMessage(p);
}

void FMessageCenter::sendReaderConnectionStatus()
{
    JPacket p(PacketType::Notification);
    p.module = "Reader";
    p.api = "ConnectionStatus";

    QJsonObject obj;
    obj.insert("isConnected", mReader->isConnected());
    obj.insert("address", mReader->address());
    obj.insert("port", mReader->port());
    p.paramsValue = obj;

    sendUIMessage(p);
}

void FMessageCenter::onReceiveReaderMessage_slot(const QJsonObject &obj)
{
//    qDebug() << "[Reader] >>" << obj;

    JPacket packet = FJsonRpcParser::decode(obj);
    if (packet.type == PacketType::Request) {
        //..
    } else if (packet.type == PacketType::Result) {
        auto request = m_WsPacketMap.take(packet.id);
        emit onReaderMessageResult_signal(packet, request);
    } else {
        qDebug() << "Unknown Reader Message:" << packet;
    }
}

void FMessageCenter::onNewReaderServerAddress_slot(const QString &address, quint16 port)
{
    if (mReader->address() != address || mReader->port() != port) {
        mReader->setServerAddressPort(address, port);

        FCommon::setConfigFileValue("reader", "websocket", "address", address);
        FCommon::setConfigFileValue("reader", "websocket", "port", port);
    }
}

void FMessageCenter::configModule(JPacket &packet)
{
    QJsonObject paramsObj = packet.paramsValue.toObject();
    JPacket p(PacketType::Result, packet.id);

    if (packet.api == "Emergency") {
        int closeTime = paramsObj.value("closeTime").toInt();
        TrackManager::GetInstance()->mEmergency->setCloseTime(closeTime);
        p.resValue = true;
    } else if (packet.api == "RecycelBox") {
        int countMax = paramsObj.value("countMax").toInt();
        StainManager::GetInstance()->mRecycleBoxes->setBoxMaxCapacity(countMax);
        p.resValue = true;
    } else if (packet.api == "StainProcess") {
        StainManager::GetInstance()->setStainParams(paramsObj);
        SmearManager::GetInstance()->mSampling->setRetHatchTime(paramsObj);
        RtSampleManager::GetInstance()->setStainProcessList(paramsObj);
        p.resValue = true;
    } else if (packet.api == "Track") {
        int waitTime = paramsObj.value("waitTime").toInt();
        TrackManager::GetInstance()->mImport->setWaitTime(waitTime);
        p.resValue = true;
    } else if (packet.api == "AddSample") {
        bool en = paramsObj.value("isUserParams").toBool();
        if (en) {
            QJsonObject addSampleParamsObj = paramsObj.value("addSampleParams").toObject();
            int addSampleValue = addSampleParamsObj.value("addSampleValue").toInt();
            SmearManager::GetInstance()->mSampling->setAddSampleVolume(addSampleValue);
        }
        p.resValue = true;
    } else if (packet.api == "SetSmearParams") {
        if (paramsObj.contains("basis")) {
            SmearManager::GetInstance()->mSmear->setSmearZhightBasisDB(paramsObj.value("basis").toInt());
            p.resValue = true;
        } else if (paramsObj.contains("params")) {
            RtSampleManager::GetInstance()->setSmearParams(packet.paramsValue.toObject());
            p.resValue = true;
        } else {
            qWarning() << "SetSmearParams Missing params.";
            p.resValue = false;
        }
    } else if (packet.api == "GetSmearParams") {
        p.resValue = RtSampleManager::GetInstance()->getSmearParams(packet.paramsValue.toObject());
    } else if (packet.api == "AutoTest") {
        QJsonObject obj = paramsObj.value("mode").toObject();
        bool isPrint = obj.value("isPrint").toBool();
        bool isSmear = obj.value("isSmear").toBool();
        bool isStain = obj.value("isStain").toBool();
        RtSampleManager::GetInstance()->setAutoTestMode(isPrint, isSmear, isStain);
        p.resValue = true;
    } else if (packet.api == "SmearDry") {
        StainManager::GetInstance()->mStainCart->setHeaterParams(paramsObj);
        p.resValue = true;
    } else if (packet.api == "StainDry") {
        StainManager::GetInstance()->mRecycleBoxes->setHeaterParams(paramsObj);
        p.resValue = true;
    } else if (packet.api == "PrintInfo") {
        if (FCommon::GetInstance()->isPrintEnable()) {
            SmearManager::GetInstance()->mSlideStore->setPrintMode(paramsObj);
        } else {
            qInfo() << "Print Module is NOT available.";
        }
        p.resValue = true;
    } else {
        qWarning() << "can NOT handle config api:" << packet.api;
    }

    if (!p.resValue.isNull()) {
        sendUIMessage(p);
    }
}
