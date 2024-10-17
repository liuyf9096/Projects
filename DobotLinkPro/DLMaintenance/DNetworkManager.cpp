#include "DNetworkManager.h"

#include <QJsonDocument>
#include <QUrlQuery>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QDebug>

const QString BaseUrl = "https://cn.dobot.cc";

DNetworkManager *DNetworkManager::getInstance()
{
    static DNetworkManager *instance = nullptr;
    if (instance == nullptr) {
        instance = new DNetworkManager();
    }
    return instance;
}

DNetworkManager::DNetworkManager(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &DNetworkManager::replyFinished_slot);

#ifdef Q_OS_WIN
    QString appPath = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    m_fileDir.setPath(appPath);
#if 0
    if (!m_fileDir.exists("download")) {
        m_fileDir.mkdir("download");
    }
    m_fileDir.cd("download");
#endif
#elif defined (Q_OS_MAC)
    QString download = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_fileDir.setPath(download);
#endif
}

DNetworkManager::~DNetworkManager()
{
    qDebug() << "DNetworkManager destroyed.";
}

QNetworkReply *DNetworkManager::downloadPlugin(QString url)
{
    QFileInfo info(BaseUrl + url);
    QString fileName = QString("{$$}%1").arg(info.fileName());
    QString downloadFileName = m_fileDir.absoluteFilePath(fileName);

    QFile *file = new QFile(downloadFileName, this);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        delete file;
        qDebug() << "download file open fail.";
        return nullptr;
    }

    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(BaseUrl + url)));
    connect(reply, &QNetworkReply::downloadProgress,
            this, [=](qint64 bytesReceived, qint64 bytesTotal){
        emit downloadProgress_signal(bytesReceived, bytesTotal, info.fileName());
    });
    file->setParent(reply);

    m_downloadReplyMap.insert(reply, file);
    return reply;
}

QNetworkReply *DNetworkManager::getDobotRequest(QString url, int timeoutMs)
{
    QNetworkReply *reply = getRequest(BaseUrl + url, timeoutMs);
    return reply;
}

QNetworkReply * DNetworkManager::getRequest(QString url, int timeoutMs)
{
    qDebug() << "[DNetworkManager] send GET Request:" << url;

    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &DNetworkManager::onReplyError_slot);
    connect(reply, &QNetworkReply::sslErrors, this, &DNetworkManager::onSslErrors_slot);

    setTimeoutRequest(reply, timeoutMs);
    return reply;
}

void DNetworkManager::cancelDownload()
{
    foreach (QNetworkReply *reply, m_downloadReplyMap.keys()) {
        reply->abort();
    }

    m_downloadReplyMap.clear();
}

// set timeout
void DNetworkManager::setTimeoutRequest(QNetworkReply *reply, int msec)
{
    if (msec == 0) {
        return;
    }
    QTimer *timer = new QTimer(reply);
    timer->setObjectName(reply->url().toString() + " timer");
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=](){
        qDebug() << "[DNetworkManager] request timeout, url:" << reply->url();
        reply->abort();
    });
    timer->start(msec);
}

// finish Slot
void DNetworkManager::replyFinished_slot(QNetworkReply *reply)
{
    QString url = reply->url().toString();
    if (url.startsWith(BaseUrl)) {
        url = url.remove(BaseUrl);
    }

    qDebug() << "[DNetworkManager Reply]" << url;

    if (m_downloadReplyMap.contains(reply)) {
        QFile *file = m_downloadReplyMap.take(reply);
        file->write(reply->readAll());
        file->close();

        emit downloadFinished_signal(url, file->fileName());
    } else {
        if (url.endsWith("DobotLink/version.json")) {
            QJsonObject resObj = parseJsonData(reply->readAll());
            emit onReplyMassage_signal(url, resObj);
        }
    }

    reply->deleteLater();
}

QJsonObject DNetworkManager::parseJsonData(QByteArray byteArray)
{
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(byteArray, &jsonError);
    QJsonObject resObject;

    if (jsonError.error == QJsonParseError::NoError) {
        if (document.isObject()) {
            resObject = document.object();
        }
    } else {
        qDebug() << "[DNetworkManager] handle ReplyData error: QJsonParseError";
    }

    return resObject;
}

void DNetworkManager::onReplyError_slot(QNetworkReply::NetworkError code)
{
    qDebug() << "[DNetworkManager] error:" << code;
}

void DNetworkManager::onSslErrors_slot(const QList<QSslError> &errors)
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    qDebug()<< "[DNetworkManager] error, url:" << reply->url() << "  ssl error:" << errors;
}
