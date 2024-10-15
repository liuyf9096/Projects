#include "jsonrpc_parser.h"

#include <QJsonDocument>
#include <QDebug>

//![PACKET]

JPacket::JPacket(PacketType type, quint64 id)
    : id(id)
    , type(type)
{
    resValue = true;
}

JPacket::JPacket(const QJsonObject &obj)
    : id(0)
    , type(PacketType::None)
    , data(obj)
{
}

QDebug operator<<(QDebug dbg, const JPacket &p)
{
    if (p.type == PacketType::Notification)
    {
        dbg.noquote() << "[notification] method:" << p.method << p.paramsValue;
    }
    else if (p.type == PacketType::Request)
    {
        QString s = QString("[request] id:%1 method:%2.%3.%4 params:").arg(p.id).arg(p.device, p.module, p.api);
        dbg.noquote() << s << p.paramsValue;
    }
    else if (p.type == PacketType::Result)
    {
        QString s = QString("[result] id:%1 result:").arg(p.id);
        dbg.noquote() << s << p.resValue;
    }
    else
    {
        dbg.noquote() << p.getSourceObj();
    }

    return dbg;
}

//![PACKET PARSER]

JPacket FJsonRpcParser::decode(const QJsonObject &obj)
{
    JPacket p(obj);

    if (obj.isEmpty()) {
        qWarning() << "[PacketParser] decode error. obj is empty";
        return p;
    }

    if (obj.contains("jsonrpc")) {
        QJsonValue jsonrpcValue = obj.value("jsonrpc");
        if (jsonrpcValue.isString()) {
            QString jsonrpc = jsonrpcValue.toString();
            if (jsonrpc != QStringLiteral("2.0")) {
                qDebug() << "[PacketParser] jsonrpc identification error.";
            }
        }
    }

    if (obj.contains("id")) {
        QJsonValue idValue = obj.value("id");
        if (idValue.isDouble()) {
            p.id = static_cast<quint64>(idValue.toDouble());
        }
    } else {
        p.type = PacketType::Notification;
    }

    if (obj.contains("result"))
    {
        p.type = PacketType::Result;
        p.resValue = obj.value("result");
    }
    else if (obj.contains("error"))
    {
        p.type = PacketType::Error;
        p.errorObj = obj.value("error").toObject();
        if (p.errorObj.contains("code")) {
            p.errorCode = p.errorObj.value("code").toInt();
        }
        if (p.errorObj.contains("message")) {
            p.errorMessage = p.errorObj.value("message").toString();
        }
    }
    else if (obj.contains("method"))
    {
        QJsonValue methodValue = obj.value("method");
        if (methodValue.isString())
        {
            if (p.type != PacketType::Notification) {
                p.type = PacketType::Request;
            }
            p.method = methodValue.toString();

            QStringList list = p.method.split(".");
            if (list.count() == 3) {
                p.device = list.at(0);
                p.module = list.at(1);
                p.api    = list.at(2);
            } else {
                p.api = p.method;
            }
        }
    }

    if (obj.contains("params")) {
       p.paramsValue = obj.value("params");
    }

    return p;
}

QJsonObject FJsonRpcParser::encode(const JPacket &p)
{
    QJsonObject resObj;
    resObj.insert("jsonrpc", QStringLiteral("2.0"));

    QString method = p.method;
    if (method.isEmpty()) {
        method = QString("%1.%2.%3").arg(p.device).arg(p.module).arg(p.api);
    }

    if (p.type == PacketType::Notification) /* method, params */
    {
        resObj.insert("method", method);
        if (!p.paramsValue.isNull()) {
            resObj.insert("params", p.paramsValue);
        }
    }
    else
    {
        resObj.insert("id", static_cast<double>(p.id));

        if (p.type == PacketType::Result)   /* id, result */
        {
            resObj.insert("result", p.resValue);
        }
        else if (p.type == PacketType::Error)   /* id, error */
        {
            if (!p.errorObj.isEmpty()) {
                resObj.insert("error", p.errorObj);
            } else {
                QJsonObject errorObj;
                if (p.errorCode != 0) {
                    errorObj.insert("code", p.errorCode);
                }
                if (!p.errorMessage.isEmpty()) {
                    errorObj.insert("message", p.errorMessage);
                }
                resObj.insert("error", errorObj);
            }
        }
        else if (p.type == PacketType::Request) /* id, method, params */
        {
            resObj.insert("method", method);
            if (!p.paramsValue.isNull()) {
                resObj.insert("params", p.paramsValue);
            }
        } else {
            qDebug() << "packet type None.";
        }
    }
    return resObj;
}

QByteArray FJsonRpcParser::convertObjToByte(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray byteArray = doc.toJson(QJsonDocument::Compact);
    return byteArray;
}

QJsonObject FJsonRpcParser::getObjFromString(const QString &str)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError) {
        return doc.object();
    }
    return QJsonObject();
}
