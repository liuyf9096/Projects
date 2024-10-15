#include "mytask.h"

#include "f_websocket_server.h"
#include "jsonrpc_parser.h"

MyTask::MyTask(QObject *parent) : QObject(parent)
{
    mServer = FWebSocketServer::GetInstance();
    connect(mServer, &FWebSocketServer::receiveMassage_signal,
            this, &MyTask::onReceiveMessage);
}

void MyTask::onReceiveMessage(const QString &client_id, const QString &message)
{
    QJsonObject obj = FJsonRpcParser::getObjFromString(message);
    JPacket p = FJsonRpcParser::decode(obj);
    JPacket res_p(PacketType::Result, p.id);
    res_p.resValue = true;

    QJsonObject resObj = FJsonRpcParser::encode(res_p);
    mServer->sendMessage(client_id, FJsonRpcParser::convertObjToByte(resObj));
}
