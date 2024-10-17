#include "Module.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QDebug>

#include "curlnetwork/CurlNetworkManager.h"

#define USE_CURL_LIB

Module::Module(QString basePort, QObject *parent) : QObject(parent), BasePort(basePort)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &Module::receivedata_slot);

#ifdef USE_CURL_LIB
    m_crulManager = new CurlNetworkManager(this);
#endif
}

bool Module::setIpAddress(QString ip)
{
    if (containsIPaddress(ip)) {
        qDebug() << "set target ip address:" << ip;
        m_ip = ip;
        return true;
    } else {
        qDebug() << "wrong ip address." << ip;
        return false;
    }
}

bool Module::readFile(const QString fileName, QByteArray &data)
{
    QString r_fileName = QString("//%1%2").arg(m_ip).arg(fileName);

    qDebug() << __FUNCTION__ << r_fileName;

    QFile file(r_fileName);
    QFileInfo info(file);
    if (info.exists()) {
        if (file.open(QFile::ReadOnly)) {
            data = file.readAll();
            file.close();
            return true;
        }
    } else {
        qDebug() << "file don't exist.";   
    }
    return false;
}

bool Module::writeFile(QString fileName, const QJsonValue &value)
{
    QString w_fileName = QString("//%1%2").arg(m_ip).arg(fileName);

    qDebug() << __FUNCTION__ << w_fileName;

    QJsonDocument doc;
    if (value.isObject()) {
        doc.setObject(value.toObject());
    } else if (value.isArray()) {
        doc.setArray(value.toArray());
    }

    QFile file(w_fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(doc.toJson());
        file.close();
        return true;
    } else {
        qDebug() << "file write fail.";
    }
    return false;
}

bool Module::writeFile(QString fileName, const QString &content)
{
    QString w_fileName = QString("//%1%2").arg(m_ip).arg(fileName);

    qDebug() << __FUNCTION__ << w_fileName;

    QFile file(w_fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(content.toUtf8());
        file.close();
        return true;
    } else {
        qDebug() << "file write fail.";
    }
    return false;
}

void Module::sendGetRequest(const QString url, const quint64 id, const QString api)
{
    QString req_url = QString("http://%1:%2%3").arg(m_ip).arg(BasePort).arg(url);

    qDebug().noquote() << QString("<< [GET] (id:%1) url:%2").arg(id).arg(req_url);

#ifdef USE_CURL_LIB
    CurlNetworkReply *reply = m_crulManager->getJson(QUrl(req_url), id);
    connect(reply, &CurlNetworkReply::finished_signal, this, &Module::receiveCurlData_slot);
#else
    QNetworkReply *reply = m_manager->get(QNetworkRequest(req_url));
#endif

    if (reply) {
        reply->setProperty("id", id);
        if (!api.isEmpty()) {
            reply->setProperty("api", api);
        }
    }
}

void Module::sendPostRequest(const QString url, const QJsonValue value, const quint64 id, const QString api)
{
    QString req_url = QString("http://%1:%2%3").arg(m_ip).arg(BasePort).arg(url);

    qDebug().noquote() << QString("<< [POST] (id:%1) url:%2").arg(id).arg(req_url) << value;

#ifdef USE_CURL_LIB
    CurlNetworkReply *reply = m_crulManager->postJson(QUrl(req_url), value, id);
    connect(reply, &CurlNetworkReply::finished_signal, this, &Module::receiveCurlData_slot);
#else
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setUrl(QUrl(req_url));

    QByteArray data_send;

    if (!value.isNull() and !value.isUndefined()) {
        QJsonDocument doc;
        if (value.isObject()) {
            doc.setObject(value.toObject());
        } else if (value.isArray()) {
            doc.setArray(value.toArray());
        }
        data_send = doc.toJson();

        request.setHeader(QNetworkRequest::ContentLengthHeader, data_send.size());
    }

    QNetworkReply *reply = m_manager->post(request, data_send);
#endif

    if (reply) {
        reply->setProperty("id", id);
        if (!api.isEmpty()) {
            reply->setProperty("api", api);
        }
    }
}

bool Module::containsIPaddress(const QString address)
{
    QRegExp rx("((25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]?\\d)");
    if (rx.indexIn(address) > -1) {
        return true;
    }
    return false;
}

/* [Receive] */
void Module::receivedata_slot(QNetworkReply *reply)
{
    QByteArray receiveData = reply->readAll();
    QString url = reply->url().toString();
    quint64 id = reply->property("id").toDouble();
    QString api = reply->property("api").toString();

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(receiveData, &jsonError);

    if (jsonError.error == QJsonParseError::NoError) {
        QJsonValue resValue;
        if (document.isObject()) {
            resValue = document.object();
        } else if (document.isArray()) {
            resValue = document.array();
        }

        qDebug().noquote() << QString(">> [REPLY] (id:%1)").arg(id) << resValue;

        if (!resValue.isNull() && !resValue.isUndefined()) {
            emit onReceiveData_signal(resValue, url, id, api);
        }
    } else {
        qDebug() << "error: QJsonParseError" << receiveData;
        emit onErrorOccured_signal(id, 666);
    }

    reply->deleteLater();
}

void Module::receiveCurlData_slot(const quint64 id, const QByteArray message)
{
    auto reply = qobject_cast<CurlNetworkReply *>(sender());
    if (reply == nullptr) {
        return;
    }

    QString api = reply->property("api").toString();

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(message, &jsonError);

    if (jsonError.error == QJsonParseError::NoError) {
        if (document.isObject()) {
            QJsonObject obj = document.object();

            qDebug().noquote() << QString(">> [REPLY] (id:%1)").arg(id) << obj;
            emit onReceiveData_signal(obj, reply->urlstr(), id, api);
        } else {
            QJsonArray arr = document.array();
            qDebug().noquote() << QString(">> [REPLY] (id:%1)").arg(id) << arr << "todo todo todo";
        }
    } else {
        qDebug() << "error: QJsonParseError" << message;
        emit onErrorOccured_signal(id, 666);
    }

    reply->deleteLater();
}

QJsonValue Module::parseJsonData(QByteArray byteArray)
{
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(byteArray, &jsonError);

    if (jsonError.error == QJsonParseError::NoError) {
        if (document.isObject()) {
            return document.object();
        } else if (document.isArray()) {
            return document.array();
        }
    } else {
        qDebug() << "error: QJsonParseError";
    }

    return QJsonValue();
}
