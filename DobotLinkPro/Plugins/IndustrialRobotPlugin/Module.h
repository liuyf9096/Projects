#ifndef MODULE_H
#define MODULE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonValue>

class QNetworkAccessManager;
class QNetworkReply;
class CurlNetworkManager;

class Module : public QObject
{
    Q_OBJECT
public:
    explicit Module(QString basePort, QObject *parent = nullptr);

    const QString BasePort;

    bool setIpAddress(QString ip);

    /* file */
    bool readFile(const QString fileName, QByteArray &data);
    bool writeFile(QString fileName, const QJsonValue &value);
    bool writeFile(QString fileName, const QString &content);

    /* get post */
    void sendGetRequest(const QString url, const quint64 id = 0, const QString api = QString());
    void sendPostRequest(const QString url, const QJsonValue value, const quint64 id = 0, const QString api = QString());

    bool containsIPaddress(const QString address);
    QJsonValue parseJsonData(QByteArray byteArray);

signals:
    void onReceiveData_signal(QJsonValue value, QString url, quint64 id, QString api);
    void onErrorOccured_signal(quint64 id, int code);

private:
    QNetworkAccessManager *m_manager;
    CurlNetworkManager *m_crulManager;

    QString m_ip;

protected slots:
    void receivedata_slot(QNetworkReply *reply);
    void receiveCurlData_slot(const quint64 id, const QByteArray message);

};

#endif // MODULE_H
