#include "DDfufile.h"

#include <QCoreApplication>
#include <QFile>
#include <QDebug>

DDfufile::DDfufile(QObject *parent) : QObject(parent)
{
    _processInit();
}

void DDfufile::_processInit()
{
    QString appPath = QCoreApplication::applicationDirPath().remove("_d");
    m_workSpaceDir.setPath(appPath);
#ifdef Q_OS_MAC
    m_workSpaceDir.cdUp();
    m_workSpaceDir.cd("Resources");
#endif
    if (m_workSpaceDir.exists("tool")) {
        m_workSpaceDir.cd("tool");
#ifdef Q_OS_WIN
        m_workSpaceDir.cd("dfu");
#elif defined (Q_OS_MAC)
        m_workSpaceDir.cd("dfu-util");
        m_workSpaceDir.cd("0.9");
        m_workSpaceDir.cd("bin");
#endif
    } else {
        qDebug() << "work space error:" << m_workSpaceDir;
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setWorkingDirectory(m_workSpaceDir.absolutePath());

    connect(m_process, &QProcess::readyRead, this, &DDfufile::onProcessReadyRead_slot);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DDfufile::onProcessFinished_slot);
    connect(m_process, &QProcess::errorOccurred, this, &DDfufile::errorHandle_slot);

    QDir dir(m_workSpaceDir);

#ifdef Q_OS_WIN
    QString program = "dfucmd.exe";
#elif defined (Q_OS_MAC)
    QString program = "dfu-util";
#endif

    if (dir.exists(program)) {
        m_process->setProgram(dir.absoluteFilePath(program));
    } else {
        qDebug() << "DDfufile:" << program << "path is not correct.";
    }
}

void DDfufile::setFileName(QString fileName)
{
    QString appPath = QCoreApplication::applicationDirPath().remove("_d");
    QDir dir(appPath);

#ifdef Q_OS_MAC
    dir.cdUp();
    dir.cd("Resources");
#endif

    if (!dir.exists("firmware")) {
        qDebug() << "dfu fileName error.";
        return;
    }
    dir.cd("firmware");

    if (fileName.isEmpty()) {
        QStringList nameFilters;
        nameFilters << "*.dfu";
        QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);

        QString typeName;
        if (m_type == DFU_MAGICBOX) {
            typeName = "MagicBox-";
        } else if (m_type == DFU_MAGICIANLITE) {
            typeName = "MagicianLite-";
        }

        foreach (QString file, files) {
            if (file.startsWith(typeName)) {
                m_fileName = dir.absoluteFilePath(file);
            }
        }
    } else if (!fileName.contains("firmware")) {
        m_fileName = dir.absoluteFilePath(fileName);
    } else {
        m_fileName = fileName;
    }
}

/* DfuSeCommand -c -d --fn ***.dfu */
void DDfufile::startDownload()
{
    QFileInfo fileinfo(m_fileName);
    if (!fileinfo.exists()) {
        qDebug() << "dfu file not exists";
        return;
    }

    QStringList arguments;
#ifdef Q_OS_WIN
    arguments << "-c" << "-d" << "--fn" << m_fileName;
#elif defined (Q_OS_MAC)
    arguments << "-a" << "0" << "-D" << m_fileName;
#endif

#ifndef QT_NO_DEBUG
    qDebug() << "arguments" << arguments << m_process->program();
#endif
    m_process->setArguments(arguments);
    m_process->start();
}

/* SLOT */
void DDfufile::onProcessReadyRead_slot()
{
    QString readStr = QString::fromLatin1(m_process->readAll());

    emit onProcessReadyRead_signal(readStr);
}

void DDfufile::onProcessFinished_slot(int exitCode, QProcess::ExitStatus exitStatus)
{
//    auto process = qobject_cast<QProcess *>(sender());

    if (exitStatus == QProcess::NormalExit) {
        emit onFinish_signal(true);
    } else {
        emit onFinish_signal(false);
        qDebug() << "error, exitCode:" << exitCode << "exitStatus:" << exitStatus;
    }
}

void DDfufile::errorHandle_slot(QProcess::ProcessError error)
{
    qDebug() << "DDfufile: QProcess ERROR:" << error;
}
