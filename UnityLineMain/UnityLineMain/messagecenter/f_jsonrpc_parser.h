#ifndef F_JSONRPC_PARSER_H
#define F_JSONRPC_PARSER_H

/****************************************************************
 * function : help you handle json object base on jsonrpc2.0    *
 *   author : liuyufei                                          *
 * datetime : 2021-09-18 09:22:41                               *
 *  website : https://www.jsonrpc.org/                          *
 *                                                              *
 * Example:                                                     *
 * Request:                                                     *
 * {                                                            *
 *    "id": 1,                                                  *
 *    "jsonrpc": "2.0",                                         *
 *    "method": "BD2000.SmearTrack.ResetAll"                    *
 *    "params": {...}                                           *
 * }                                                            *
 * Reply:                                                       *
 * {                                                            *
 *    "id": 1,                                                  *
 *    "jsonrpc": "2.0",                                         *
 *    "result": true                                            *
 * }                                                            *
 ****************************************************************/

#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>

enum class PacketType { None, Request, Result, Notification, Error };

class JPacket
{
    friend class PacketParser;
public:
    JPacket(PacketType type = PacketType::None, quint64 id = 0);
    explicit JPacket(const QJsonObject &obj);

    quint64 id;

    PacketType type;

    /* method: "device . module . api" */
    QString method;
    QString device;
    QString module;
    QString api;
    QString subapi;

    /* params: value */
    QJsonValue paramsValue;

    /* result: jsonValue(bool/obj/array) */
    QJsonValue resValue;

    /* errorObj : (code + message) */
    QJsonObject errorObj;
    int errorCode;
    QString errorMessage;

    QJsonObject getSourceObj() const { return data; }

private:
    QJsonObject data;
};

QDebug operator<<(QDebug, const JPacket &);

Q_DECLARE_METATYPE(JPacket)

class FJsonRpcParser
{
public:
    static JPacket decode(const QJsonObject &obj);              /* jsonObj -> packet   */
    static QJsonObject encode(const JPacket &p);                /* packet  -> jsonObj  */

    static QByteArray convertObjToByte(const QJsonObject &obj); /* jsonObj -> byteArray*/
    static QJsonObject getObjFromString(const QString &str);    /* string  -> jsonObj  */
};

#endif // F_JSONRPC_PARSER_H
