#ifndef DDFUFILE_H
#define DDFUFILE_H

#include <QObject>
#include <QProcess>
#include <QDir>

class DDfufile : public QObject
{
    Q_OBJECT
public:
    explicit DDfufile(QObject *parent = nullptr);

    enum DeviceType {
        DFU_MAGICBOX,
        DFU_MAGICIANLITE
    } m_type;

    void setFileName(QString fileName);
    void startDownload();

signals:
    void onProcessReadyRead_signal(QString message);
    void onFinish_signal(bool ok);

private:
    QDir m_workSpaceDir;
    QString m_fileName;

    QProcess *m_process;

    void _processInit();
    void _codefileInit();

private slots:
    void onProcessReadyRead_slot(void);
    void onProcessFinished_slot(int exitCode, QProcess::ExitStatus exitStatus);
    void errorHandle_slot(QProcess::ProcessError error);
};

#endif // DDFUFILE_H
