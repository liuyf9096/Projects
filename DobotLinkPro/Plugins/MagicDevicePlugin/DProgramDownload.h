#ifndef DPROGRAMDOWNLOAD_H
#define DPROGRAMDOWNLOAD_H

#include <QObject>
#include <QJsonObject>
#include <QDir>
#include <QFile>

class DProgramDownload : public QObject
{
    Q_OBJECT
public:
    explicit DProgramDownload(QObject *parent = nullptr);

    bool searchDeviceDrive();
    bool writeToTempFile(QString text);
    bool copyTempToMagicBox();

    void handleProgramRequest(QString fileName, QString code, quint64 id);

signals:
    void onDownloadFinished_signal(quint64 id, bool isOk, QJsonObject errorObj = QJsonObject());

private:
    QDir m_localTempDir;
    QDir m_boxDir;

    QFile m_codePyFile;
    QFile m_boxPyFile;

    quint64 m_handleID;
    bool m_isDownloading;

};

#endif // DPROGRAMDOWNLOAD_H
