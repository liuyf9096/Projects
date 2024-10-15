#ifndef F_NETWORK_SERVER_H
#define F_NETWORK_SERVER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QFile>

class FNetworkServer : public QObject
{
    Q_OBJECT
public:
    static FNetworkServer *GetInstance();

    void showIPAddress();
    QStringList getIPAddress();

private:
    explicit FNetworkServer(QObject *parent = nullptr);
    ~FNetworkServer();

    QNetworkAccessManager *m_manager;
};

#endif // F_NETWORK_SERVER_H
