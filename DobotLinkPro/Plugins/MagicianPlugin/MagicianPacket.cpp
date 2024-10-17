#include "MagicianPacket.h"

#include <QStringList>

const QString JsonRPCVersion = "2.0";
const int ErrorBaseIndex = 100;

MagicianPacket::MagicianPacket()
{

}

bool MagicianPacket::setPacket(QJsonObject obj)
{
    if (obj.contains("WSport")) {
        port = static_cast<quint16>(obj.value("WSport").toInt());
    }

    id = obj.value("id").toDouble(-1);

    if (obj.contains("method")) {
        QJsonValue value = obj.value("method");
        if (value.isString()) {
            method = value.toString();
            if (method.startsWith("dobotlink")) {
                QStringList methodList = method.split(".", QString::SkipEmptyParts);
                if (methodList.count() >= 2) {
                    target = methodList.at(1);
                }
                if (methodList.count() >= 3) {
                    api = methodList.at(2);
                }
            }
        }
    }

    if (obj.contains("params")) {
        QJsonValue value = obj.value("params");
        if (value.isObject()) {
            paramsObj = value.toObject();
            if (paramsObj.contains("portName")) {
                portName = paramsObj.value("portName").toString();
            }
            /* maybe it contains 'data' 'type' etc. */
        }
    }

    if (api.isEmpty()) {
        return false;
    }
    return true;
}

QJsonObject MagicianPacket::getNotificationObj()
{
    QJsonObject resObj;
    resObj.insert("WSport", port);
    resObj.insert("jsonrpc", JsonRPCVersion);
    resObj.insert("method", method);
    if (!paramsObj.isEmpty()) {
        resObj.insert("params", paramsObj);
    }
    return resObj;
}

MagicianResPacket::MagicianResPacket(double id, quint16 port) : m_id(id), m_port(port)
{
    resObj.insert("WSport", port);
    resObj.insert("id", id);
    resObj.insert("jsonrpc", JsonRPCVersion);
}

void MagicianResPacket::setResultObj(QJsonValue data)
{
    m_resultVal = data;
}

void MagicianResPacket::setErrorObj(int code, QString message)
{
    if (code != 0) {
        errorObj.insert("code", ErrorBaseIndex + code);
    }
    if (!message.isEmpty()) {
        errorObj.insert("message", message);
    }
}

QJsonObject MagicianResPacket::getResultObj()
{
    if (errorObj.isEmpty()) {
        resObj.insert("result", m_resultVal);
    } else {
        resObj.insert("error", errorObj);
    }
    return resObj;
}


