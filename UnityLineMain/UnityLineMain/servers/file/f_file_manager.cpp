#include "f_file_manager.h"
#include "f_common.h"

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>

QByteArray FFileManager::readFile(const QString &fileName, FileLocation loc)
{
    if (fileName.isEmpty()) {
        qWarning() << "FileName is empty.";
        return QByteArray();
    }

    QString ab_fileName;

    if (loc == POS_APP) {
        QDir dir(FCommon::appPath());
        ab_fileName = dir.absoluteFilePath(fileName);
    } else if (loc == POS_CONFIG) {
        QDir dir(FCommon::getPath("config"));
        ab_fileName = dir.absoluteFilePath(fileName);
    } else {
        ab_fileName = fileName;
    }

    qDebug() << "read file:" << ab_fileName;

    QFile file(ab_fileName);
    bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(ok == true) {
        QByteArray content = file.readAll();
        return content;
    } else {
        qWarning().noquote() << QString("File (%1) open error.").arg(ab_fileName);
        return QByteArray();
    }
}

QJsonObject FFileManager::readJsonFileObj(const QString &fileName, FileLocation loc)
{
    QString fname = fileName;
    if (!fname.endsWith(".json")) {
        fname.append(".json");
    }
    QByteArray data = readFile(fname, loc);
    QJsonDocument doc(QJsonDocument::fromJson(data));
    return doc.object();
}

QJsonObject FFileManager::readBinaryFileObj(const QString &fileName, FileLocation loc)
{
    QByteArray data = readFile(fileName, loc);
    QJsonDocument doc(QJsonDocument::fromBinaryData(data));
    return doc.object();
}

bool FFileManager::writeJsonFileObj(const QJsonObject &obj, const QString &fileName, FileLocation loc)
{
    QString fname = fileName;
    if (!fname.endsWith(".json")) {
        fname.append(".json");
    }
    QJsonDocument doc(obj);
    QByteArray byteArray = doc.toJson(QJsonDocument::Indented);
    return writeFile(fname, byteArray, loc);
}

bool FFileManager::writeFile(const QString &fileName, const QByteArray &data, FileLocation loc)
{
    if (fileName.isEmpty()) {
        qWarning() << "FileName is empty.";
        return false;
    }

    if (data.isEmpty()) {
        qWarning() << "File data is empty.";
        return false;
    }

    QString ab_fileName;
    if (loc == POS_APP) {
        QDir dir(FCommon::appPath());
        ab_fileName = dir.absoluteFilePath(fileName);
    } else if (loc == POS_CONFIG) {
        QDir dir(FCommon::getPath("config"));
        ab_fileName = dir.absoluteFilePath(fileName);
    } else {
        ab_fileName = fileName;
    }

    qDebug() << "write file:" << ab_fileName;

    QFile file(ab_fileName);
    bool ok = file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    if(ok == true) {
        file.write(data);
        file.close();
        return true;
    } else {
        qWarning().noquote() << QString("File (%1) open error.").arg(ab_fileName);
        return false;
    }
}

bool FFileManager::writeFile(const QString &fileName, const QString &text, FileLocation loc)
{
    QByteArray data;
    data.append(text);

    return writeFile(fileName, data, loc);
}
