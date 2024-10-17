#ifndef CURLNETWORKMANAGER_H
#define CURLNETWORKMANAGER_H

#include <QObject>
#include <QThreadPool>
#include <QJsonValue>
#include <QUrl>

#include "CurlNetworkRequest.h"
#include "CurlNetworkReply.h"

class CurlNetworkReply;
class CurlNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit CurlNetworkManager(QObject *parent = nullptr);
    ~CurlNetworkManager();

    CurlNetworkReply *get(QUrl url, quint64 id = 0);
    CurlNetworkReply *getJson(QUrl url, quint64 id = 0);
    CurlNetworkReply *post(QUrl url, QByteArray data, quint64 id = 0);
    CurlNetworkReply *postJson(QUrl url, const QJsonValue &value, quint64 id = 0);
    CurlNetworkReply *downloadFile(CurlNetworkRequest &request, QString savePath, bool autoResume = true);

private:
    QThreadPool m_pool;
};

#endif // CURLNETWORKMANAGER_H
