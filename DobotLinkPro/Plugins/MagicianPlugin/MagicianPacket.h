#ifndef MAGICIANPACKET_H
#define MAGICIANPACKET_H

#include <QString>
#include <QJsonObject>
#include <QJsonValue>

class MagicianPacket
{
public:
    MagicianPacket();

public:
    quint16 port;

    double id;
    QString method;
    QString target;
    QString api;

    QJsonObject paramsObj;
    QString portName;

    bool setPacket(QJsonObject obj);
    QJsonObject getNotificationObj();
};

class MagicianResPacket
{
public:
    MagicianResPacket(double id = 0, quint16 port = 0);

public:
    QJsonObject resObj;
    QJsonObject errorObj;
    QJsonValue m_resultVal;

    void setResultObj(QJsonValue data);
    void setErrorObj(int code, QString message = QString());

    QJsonObject getResultObj();

private:
    double m_id;
    quint16 m_port;
};


#endif // MAGICIANPACKET_H
