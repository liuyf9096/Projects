#ifndef DNETWORKMANAGER_H
#define DNETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QFile>

class DNetworkManager : public QObject
{
    Q_OBJECT
public:
    static DNetworkManager *getInstance();

    QNetworkReply *downloadPlugin(QString url);

    QNetworkReply *getDobotRequest(QString url, int timeoutMs = 0);
    QNetworkReply *getRequest(QString url, int timeoutMs = 0);

    void cancelDownload();

signals:
    void onReplyMassage_signal(QString url, QJsonObject resObj);
    void downloadProgress_signal(qint64 bytesReceived, qint64 bytesTotal, QString fileName);
    void downloadFinished_signal(QString url, QString fileName);

private:
    explicit DNetworkManager(QObject *parent = nullptr);
    ~DNetworkManager();

    QNetworkAccessManager *m_manager;
    QMap<QNetworkReply *, QFile *> m_downloadReplyMap;
    QDir m_fileDir;

    void setTimeoutRequest(QNetworkReply *reply, int msec = 3000);
    QJsonObject parseJsonData(QByteArray byteArray);

private slots:
    /* manager slot */
    void replyFinished_slot(QNetworkReply* reply);

    /* reply slot */
    void onReplyError_slot(QNetworkReply::NetworkError code);
    void onSslErrors_slot(const QList<QSslError> &errors);
};

#endif // DNETWORKMANAGER_H
