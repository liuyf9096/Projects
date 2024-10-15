#include "rt_init.h"
#include "f_common.h"
#include "log/f_log_server.h"
#include "sql/f_sql_database_manager.h"
#include "network/f_network_server.h"
#include "websocket/f_websocket_manager.h"
#include "websocket/f_websocket_server.h"
#include "module/module_manager.h"
#include "device/rt_device_manager.h"

#include <QObject>
#include <QString>
#include <QDebug>

void servers_init()
{
    FLogServer::GetInstance();
    FCommon::GetInstance();

    /* log */
    setLog();
    FCommon::GetInstance()->printConfigInfo();

    /* SQL: config.db, record.db, unityline_log.db */
    auto SQLManager = FSqlDatabaseManager::GetInstance();
    SQLManager->addDatabase(QDir(FCommon::getPath("config")).absoluteFilePath("config.db"), "config");
    SQLManager->addDatabase(QDir(FCommon::getPath("config")).absoluteFilePath("record.db"), "record");
    qDebug() << " ";

    /* network */
    FNetworkServer::GetInstance()->showIPAddress();

    /* Websocket-Client: Canbus, DMU */
    setCanbusWebsocket();
    setDMUWebsocket();

    /* Websocket-Server: UI,S1,S2,S3 */
    setWebsoecketServer();

    /* device + module */
    RtDeviceManager::GetInstance();
    DModuleManager::GetInstance();
    RtSampleManager::GetInstance();
}

void setLog()
{
    int index = 0;
    FSqlDatabaseManager::GetInstance()->addDatabase(QDir(FCommon::getPath("log")).absoluteFilePath("unityline_log.db"), "log");
    auto logDb = FSqlDatabaseManager::GetInstance()->getDatebase("log");
    if (logDb) {
        QJsonArray arr = logDb->selectRecord("log_file", QJsonObject({{"software", "main"}}));
        if (arr.count() > 0) {
            QJsonObject obj = arr.first().toObject();
            index = obj.value("file_index").toInt();
        } else {
            logDb->insertRecord("log_file", {{"software", "main"}});
        }
    } else {
        qFatal("unityline_log.db can NOT operate.");
    }

    QJsonObject logObj = FCommon::GetInstance()->getConfigValue("log").toObject();
    bool enable = logObj.value("enable").toBool();
    bool save_text_en = logObj.value("save_text_en").toBool();
    int file_max_count = logObj.value("file_max_count").toInt();
    if (enable && FCommon::GetInstance()->isVerbose() == false) {
        auto logger = FLogServer::GetInstance();
        logger->setMaxLogFileCount(file_max_count);
        if (save_text_en == true) {
            index++;
            QString fileName = logger->saveToTxtEnable(true, index);

            QJsonObject setObj;
            setObj.insert("file_index", index);
            setObj.insert("path", fileName);
            setObj.insert("version", FCommon::appVersion());
            logDb->updateRecord("log_file", setObj, QJsonObject({{"software", "main"}}));
        }

        logger->setMessageFlags(FLogServer::TimeTypeContent);
        logger->start();
    }
}

void setCanbusWebsocket()
{
    FWebSocket *socket = FWebSocketManager::GetInstance()->addSocket("Canbus");
    QJsonObject obj = FCommon::GetInstance()->getConfigValue("canbus").toObject();
    bool enable = obj.value("enable").toBool();
    if (enable) {
        QJsonObject wsObj = obj.value("websocket").toObject();
        QString address = wsObj.value("address").toString();
        quint16 port = wsObj.value("port").toInt();
        int reconnect_interval = wsObj.value("reconnect_interval").toInt();

        socket->setServerAddressPort(address, port);
        socket->setAutoReconnectInterval(reconnect_interval);
        socket->connectServer(true, "Canbus");
    }
}

void setDMUWebsocket()
{
    FWebSocket *socket = FWebSocketManager::GetInstance()->addSocket("DMU");
    QJsonObject obj = FCommon::GetInstance()->getConfigValue("DMU").toObject();
    bool enable = obj.value("enable").toBool();
    if (enable) {
        QJsonObject wsObj = obj.value("websocket").toObject();
        QString address = wsObj.value("address").toString();
        quint16 port = wsObj.value("port").toInt();
        int reconnect_interval = wsObj.value("reconnect_interval").toInt();
        bool keep_alive_en = wsObj.value("keep_alive_en").toBool();
        int keep_alive_count = wsObj.value("keep_alive_count").toInt(10);
        bool isAutoDetect = wsObj.value("isAutoDetect").toBool();
        QString autodetect_key = wsObj.value("autodetect_key").toString();

        socket->setServerAddressPort(address, port);
        socket->setAutoReconnectInterval(reconnect_interval);
        socket->connectServer(true, "DMU");
        if (keep_alive_en) {
            socket->checkAlive(true, keep_alive_count);
        }
        if (isAutoDetect) {
            socket->autoDetectServer(autodetect_key);
        }
    }
}

void setWebsoecketServer()
{
    QJsonObject obj = FCommon::GetInstance()->getConfigValue("websocket_server").toObject();
    quint16 port = obj.value("port").toInt();
    bool send_UDP = obj.value("send_UDP").toBool();
    QString UDP_key = obj.value("UDP_key").toString();
    bool keep_alive_en = obj.value("keep_alive_en").toBool();
    int keep_alive_count = obj.value("keep_alive_count").toInt(10);

    if (send_UDP) {
        FWebSocketServer::GetInstance()->listen(port, UDP_key);
    } else {
        FWebSocketServer::GetInstance()->listen(port);
    }
    FWebSocketServer::GetInstance()->checkAlive(keep_alive_en, keep_alive_count);
}
