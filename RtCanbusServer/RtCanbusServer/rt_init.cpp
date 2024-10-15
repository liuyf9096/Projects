#include "rt_init.h"
#include "f_common.h"

#include "settings/f_settings.h"
#include "log/f_log_server.h"
#include "sql/f_sql_database_manager.h"
#include "websocket/f_websocket_server.h"
#include "device/rt_device_manager.h"
#include "canbus/f_canbus_server.h"
#include "tasks/upgrader.h"

void servers_init()
{
    FLogServer::GetInstance();

    /* setting */
    auto setting = FSettings::GetInstance();

    /* log */
    bool enRecord = setting->isLogServerEnable();
    bool verbose = FCommon::GetInstance()->isVerbose();
    if (enRecord && (verbose == false)) {
        auto logger = FLogServer::GetInstance();
        logger->setMaxLogFileCount(setting->MaxLogFileCount());
        logger->saveToTxtEnable(setting->isSaveLogToText());
        logger->setMessageFlags(FLogServer::TimeTypeContent);
        logger->start();
    }

    /* SQL: config.db, rtcanbusserver_log.db, program_syntax.db */
    auto SQLManager = FSqlDatabaseManager::GetInstance();
    SQLManager->addDatabase(QDir(FCommon::getPath("config")).absoluteFilePath("config.db"), "config");
    SQLManager->addDatabase(QDir(FCommon::getPath("log")).absoluteFilePath("rtcanbusserver_log.db"), "log");
    SQLManager->addDatabase(QDir(FCommon::getPath("config")).absoluteFilePath("program_syntax.db"), "upgrade");

    /* canbus */
    FCanbusServer::GetInstance()->setVerbose(setting->getCanbusVerbose());

    /* websocket */
    quint16 port = setting->getWebSocketPort();
    FWebSocketServer::GetInstance()->listen(port);

    /* device */
    RtDeviceManager::GetInstance();

    /* upgrade */
//    Upgrader::GetInstance()->upgradeBoards();
}

