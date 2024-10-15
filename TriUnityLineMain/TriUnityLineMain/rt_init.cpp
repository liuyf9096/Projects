#include "rt_init.h"
#include "f_common.h"

#include "settings/f_settings.h"
#include "log/f_log_server.h"
#include "sql/f_sql_database_manager.h"
#include "network/f_network_server.h"
#include "websocket/f_websocket_manager.h"
#include "websocket/f_websocket_server.h"
#include "device/rt_device_manager.h"
#include "module/module_manager.h"

#include <QObject>
#include <QString>
#include <QDebug>

void servers_init()
{
    FLogServer::GetInstance();

    /* setting (.ini) */
    auto setting = FSettings::GetInstance();

    /* log */
    bool enRecord = setting->isRecordLogEnable();
    bool debugMode = FCommon::GetInstance()->isDebugMode();
    if (enRecord && (debugMode == false)) {
        auto logger = FLogServer::GetInstance();
        logger->saveToTxtEnable(setting->isRecordLogToText());
        logger->setMessageFlags(FLogServer::TimeTypeContent);
        logger->start();
        setting->setLogPath(logger->getLogFilePath());
    } else {
        setting->setLogPath("None");
    }

    /* sql:config */
    QDir config_dir(FCommon::configPath());
    FSqlDatabaseManager::GetInstance()->addDatabase(config_dir.absoluteFilePath("config.db"), "config");

    /* sql:log */
    QDir log_dir(FCommon::logPath());
    FSqlDatabaseManager::GetInstance()->addDatabase(log_dir.absoluteFilePath("triunityline_log.db"), "log");

    /* network */
    FNetworkServer::GetInstance()->showIPAddress();

    /* ws: IPU */
    FWebSocket *ws_ipu = FWebSocketManager::GetInstance()->addSocket("IPU");
    ws_ipu->setServerAddressPort(setting->ipu_server_addr(), setting->ipu_server_port());
    if (setting->isConnectIPU()) {
        ws_ipu->connectServer(true, "IPU");
//        ws_ipu->checkAlive(true, 5);
        ws_ipu->autoDetectServer("IPU");
    }

    /* ws: Canbus */
    FWebSocket *ws_canbus = FWebSocketManager::GetInstance()->addSocket("Canbus");
    ws_canbus->setServerAddressPort(setting->canbus_server_addr(), setting->canbus_server_port());
    ws_canbus->connectServer(true, "Canbus");

    /* Server: S1,S2,S3 */
    auto WsServer = FWebSocketServer::GetInstance();
    WsServer->listen(setting->ws_server_port(), "United");
    WsServer->checkAlive(true, 10);

    /* device */
    RtDeviceManager::GetInstance();

    /* module */
    DModuleManager::GetInstance();
}
