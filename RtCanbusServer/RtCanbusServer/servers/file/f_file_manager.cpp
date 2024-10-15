#include "f_file_manager.h"
#include "f_common.h"

#include <QFile>
#include <QDir>
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
