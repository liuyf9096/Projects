#include "CurlNetworkManager.h"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "curl/curl.h"

CurlNetworkManager::CurlNetworkManager(QObject *parent) : QObject(parent)
{
    m_pool.setMaxThreadCount(10);

    curl_global_init(CURL_GLOBAL_ALL);
}

CurlNetworkManager::~CurlNetworkManager()
{
    curl_global_cleanup();
}

CurlNetworkReply *CurlNetworkManager::get(QUrl url, quint64 id)
{
    CurlNetworkRequest request(id);
    request.setUrl(url);

    CurlNetworkReply *reply = new CurlNetworkReply(GET, request);

    qDebug() << "#[GET]:" << url.toString();

    m_pool.start(reply);
    return reply;
}

CurlNetworkReply *CurlNetworkManager::getJson(QUrl url, quint64 id)
{
    CurlNetworkRequest request(id);
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Accept", "*/*");

    CurlNetworkReply *reply = new CurlNetworkReply(GET, request);

//    qDebug() << "#[GET]:" << url.toString();

    m_pool.start(reply);
    return reply;
}

CurlNetworkReply *CurlNetworkManager::post(QUrl url, QByteArray data, quint64 id)
{
    CurlNetworkRequest request(id);
    request.setUrl(url);
    request.setBodyData(data);

    CurlNetworkReply *reply = new CurlNetworkReply(POST, request);

//    qDebug() << "#[POST]:" << url.toString() << data;

    m_pool.start(reply);
    return reply;
}

CurlNetworkReply *CurlNetworkManager::postJson(QUrl url, const QJsonValue &value, quint64 id)
{
    CurlNetworkRequest request(id);
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Accept", "*/*");

    if (!value.isNull() and !value.isUndefined()) {

        QJsonDocument doc;
        if (value.isArray()) {
            doc.setArray(value.toArray());
        } else {
            doc.setObject(value.toObject());
        }

        QString doc_str = doc.toJson();
        QByteArray doc_ba = doc_str.simplified().toLatin1();
        request.setBodyData(doc_ba);
    }

    CurlNetworkReply *reply = new CurlNetworkReply(POST, request);

//    qDebug() << "#[POST]:" << url.toString() << doc_ba;

    m_pool.start(reply);
    return reply;
}

CurlNetworkReply *CurlNetworkManager::downloadFile(CurlNetworkRequest &request, QString savePath, bool autoResume)
{
    CurlNetworkReply *reply = new CurlNetworkReply(DOWNLOAD, request);
//    reply->setFilelocalPath(QDir::toNativeSeparators(savePath));
//    reply->setAutoResume(autoResume);

    qDebug() << __FUNCTION__ << savePath << autoResume;

    m_pool.start(reply);
    return nullptr;
}
