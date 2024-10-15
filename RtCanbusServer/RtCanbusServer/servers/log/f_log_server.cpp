#include "f_log_server.h"
#include "f_log_server_p.h"
#include "f_common.h"
#include "servers/settings/f_settings.h"

#include <QTextStream>
#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QThread>
#include <QDebug>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QJsonObject obj;
    QJsonObject contextObj;
    contextObj.insert("file", context.file);
    contextObj.insert("line", context.line);
    contextObj.insert("function", context.function);
    contextObj.insert("category", context.category);

    obj.insert("context", contextObj);
    obj.insert("message", message);

    /* { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg } */
    QString type_s;
    switch (type) {
    case QtDebugMsg:    type_s = "debug";   break;
    case QtWarningMsg:  type_s = "warning"; break;
    case QtCriticalMsg: type_s = "critical";break;
    case QtFatalMsg:    type_s = "fatal";   break;
    case QtInfoMsg:     type_s = "info";    break;
    }
    obj.insert("type", type_s);

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    obj.insert("time", time);

    emit FLogServer::GetInstance()->logMessage_signal(obj);
}


//![RtLogServer]
FLogServer *FLogServer::GetInstance()
{
    static FLogServer *instance = nullptr;
    if (instance == nullptr) {
        instance = new FLogServer();
    }
    return instance;
}

FLogServer::FLogServer(QObject *parent)
    : QObject(parent)
    , Dptr(new FLogServerPrivate(this))
{

}

void FLogServer::start()
{
    Q_D(FLogServer);
    if (d->m_isLogging == true) {
        return;
    } else {
        d->m_isLogging = true;
        qInstallMessageHandler(myMessageOutput);
        d->removeRedundantLogFile();
    }
}

void FLogServer::stop()
{
    Q_D(FLogServer);
    d->m_isLogging = false;

    qDebug() << "Stop logging.";
    qInfo() << "-- THE END --";

    qInstallMessageHandler(nullptr);
}

void FLogServer::setMessageFlags(MessageFileFlags flags)
{
    Q_D(FLogServer);
    d->m_flags = flags;
}

FLogServer::MessageFileFlags FLogServer::MessageFlags()
{
    Q_D(FLogServer);
    MessageFileFlags flags = static_cast<MessageFileFlags>(d->m_flags);

    return flags;
}

QString FLogServer::getLogFilePath()
{
    Q_D(FLogServer);
    return d->mLogFile.fileName();
}

void FLogServer::setMaxLogFileCount(uint count)
{
    Q_D(FLogServer);
    if (count > 0) {
        d->m_maxCount = count;
        qDebug() << "[FLogServer] setMaxLogFileCount:" << count;
    }
}

bool FLogServer::saveToTxtEnable(bool en)
{
    Q_D(FLogServer);
    if (en == true) {
        if (!d->mLogFile.isOpen()) {
            if (d->mLogFile.fileName().isEmpty()) {
                d->creatLogFile();
            }
            bool ok = d->mLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
            if (ok) {
                connect(this, &FLogServer::logMessage_signal,
                        Dptr, &FLogServerPrivate::handleOriginalMessage_slot, Qt::QueuedConnection);
                qInfo() << "[RtLogServer] Start log at path:" << d->mLogFile.fileName();
                return true;
            } else {
                qWarning() << "[RtLogServer]log txt file open Fail." << d->mLogFile.fileName();
                return false;
            }
        } else {
            return true;
        }
    } else {
        disconnect(this, &FLogServer::logMessage_signal,
                   Dptr, &FLogServerPrivate::handleOriginalMessage_slot);
        d->mLogFile.close();
    }

    return false;
}

void FLogServer::setLogFileTitle(const QString &title)
{
    Q_D(FLogServer);
    d->m_logFileTitle = title;
}

void FLogServer::setLogFilePath(const QDir &dir)
{
    Q_UNUSED(dir)
}

void FLogServer::openLogFile()
{
    Q_D(FLogServer);
    QFileInfo fileInfo(d->mLogFile.fileName());
    if (fileInfo.exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    } else {
        qWarning() << "log file is Not exist.";
    }
}

bool FLogServer::openLogDirectory()
{
    Q_D(FLogServer);
    QFileInfo info(d->mLogFile);
    bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    return ok;
}


//![RtLogServerPrivate]
FLogServerPrivate::FLogServerPrivate(FLogServer *parent) : q_ptr(parent)
{
    m_maxCount = 100;
    m_flags = FLogServer::TimeTypeContent;
    m_isLogging = false;
    m_logFileTitle = "RTCan";

    mThread = new QThread();
    connect(mThread, &QThread::finished, mThread, &QThread::deleteLater);
    this->moveToThread(mThread);
    mThread->start();
}

FLogServerPrivate::~FLogServerPrivate()
{
    mThread->quit();
    mThread->wait();
    if (mThread->isFinished()) {
        qInfo() << "RtLogServer thread quit finished.";
    } else {
        qWarning() << "RtLogServer thread can not quit. timeout.";
    }
}

#if 0
QString appPath = QCoreApplication::applicationDirPath(); //app 路径
QString otherPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation); //系统路径
QString otherPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
SetFileAttributes((LPCWSTR)(filePath.unicode()), FILE_ATTRIBUTE_HIDDEN);    //#include "windows.h" 隐藏文件
#endif

void FLogServerPrivate::creatLogFile()
{
    QDir dir(FCommon::getPath("log"));
    if (dir.exists()) {
        uint index = FSettings::GetInstance()->logindex();
        QString dataTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QString fileName = QString("%1_%2_%3.log").arg(m_logFileTitle).arg(index, 5, 10, QChar('0')).arg(dataTime);
        mLogFile.setFileName(dir.absoluteFilePath(fileName));
    } else {
        qWarning() << "log file create fail.";
    }
}

void FLogServerPrivate::removeRedundantLogFile()
{
    QDir dir(FCommon::getPath("log"));

    QFileInfoList list = dir.entryInfoList(QStringList() << QString("%1_*.log").arg(m_logFileTitle),
                                           QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

    quint64 volume = 0;
    foreach (auto info, list) {
        volume += info.size();
    }
    qDebug().noquote() << "Log file Total size:" << QString("%1 kb (%2 Mb)").arg(volume/1024).arg(volume/1024/1024);

    if (list.count() > m_maxCount) {
        int deleteCount = list.count() - m_maxCount;
        qDebug() << "Log file has redundant files. max count:" << m_maxCount << "remove:" << deleteCount;

        quint64 reVolume = 0;
        for (int i = 0; i < deleteCount; ++i) {
            QFileInfo finfo = list.at(i);
            QFile file(finfo.absoluteFilePath());
            bool ok = file.remove();
            if (ok) {
                int size = finfo.size()/1024;
                reVolume += finfo.size();
                qInfo() << "Delete file:" << finfo.absoluteFilePath() << "size:" << size << "kb";
            }
        }
        qDebug() << "Log Release size:" << reVolume/1024 << "kb";
    }
}

/* Caution: forbid recording any message! */
void FLogServerPrivate::saveMsgToFile(const QString &type, QJsonObject context, const QString &message, const QString &time)
{
    if (!mLogFile.isOpen()) {
        return;
    }

    QString msgType;
    if (type == "debug") {
        msgType = "Debug  ";
    } else if (type == "warning") {
        msgType = "Warning";
    } else if (type == "critical") {
        msgType = "Critical";
    } else if (type == "fatal") {
        msgType = "Fatal  ";
    } else if (type == "info") {
        msgType = "Info   ";
    }

    QString text;
    if (m_flags & 0x02) {
        text.append(msgType);
    }
    if (m_flags & 0x04) {
        text.append("|");
        text.append(time);
    }
    if (m_flags & 0x01) {
        text.append("| ");
        text.append(message);
    }
    if (m_flags & 0x08) {
        text.append(" | file:");
        text.append(context.value("file").toString());
    }
    if (m_flags & 0x10) {
        text.append(" | line:");
        text.append(QString::number(context.value("line").toInt()));
    }
    if (m_flags & 0x20) {
        text.append(" | function:");
        text.append(context.value("function").toString());
    }
    if (m_flags & 0x40) {
        text.append(" | category:");
        text.append(context.value("category").toString());
    }
    text.append("\n");

    QTextStream textStream(&mLogFile);
    textStream << text.toUtf8();
    mLogFile.flush();
}

/* Caution: forbid recording any message! */
void FLogServerPrivate::handleOriginalMessage_slot(QJsonObject obj)
{
    if (!mLogFile.isOpen()) {
        return;
    }

    QJsonObject contextObj = obj.value("context").toObject();
    QString message = obj.value("message").toString();
    QString type = obj.value("type").toString();
    QString time = obj.value("time").toString();

    saveMsgToFile(type, contextObj, message, time);
}

