#include "f_common.h"

#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QDebug>

static const QString AppName = "RtCanbusServer";

FCommon *FCommon::GetInstance()
{
    static FCommon *instance = nullptr;
    if (instance == nullptr) {
        instance = new FCommon();
    }
    return instance;
}

FCommon::FCommon(QObject *parent) : QObject(parent), m_verbose(false)
{}

void FCommon::printSystemInfo()
{
    qDebug().noquote() << "\033[1;32;40m" << QString("[%1] version: %2, Author: liuyufei, release: %3")
                          .arg(FCommon::appName(), FCommon::appVersion(), FCommon::releaseDate())
                       << "\033[0m" << endl;
}

QString FCommon::appName()
{
    return AppName;
}

QString FCommon::appVersion()
{
    return QString("%1.%2.%3").arg(Version_major).arg(Version_minor).arg(Version_patch);
}

QString FCommon::appFullVersion()
{
    return QString("V%1.%2.%3")
            .arg(Version_major, 2, 10, QChar('0'))
            .arg(Version_minor, 2, 10, QChar('0'))
            .arg(Version_patch, 4, 10, QChar('0'));
}

QString FCommon::releaseDate()
{
    QString str;
    str.append(__DATE__);
    str.append(" ");
    str.append(__TIME__);

    QDateTime datetime = QDateTime::fromString(str, "MMM d yyyy hh:mm:ss");
    if (datetime.isNull()) {
        datetime = QDateTime::fromString(str, "MMM dd yyyy hh:mm:ss");
        if (datetime.isNull()) {
            return str;
        }
    }
    return datetime.toString("yyyy/MM/dd hh:mm:ss");
}

QString FCommon::appPath()
{
    QString path = QCoreApplication::applicationDirPath().remove(QRegExp("_d$"));
    return path;
}

QString FCommon::getPath(const QString &dirName)
{
    QDir dir(appPath());
    if (!dir.exists(dirName)) {
        dir.mkdir(dirName);
    }
    dir.cd(dirName);
    return dir.absolutePath();
}

QString FCommon::checkIpAddress(const QString &address)
{
    QRegExp rx("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
    if (rx.indexIn(address) > -1) {
        QString ip = rx.cap();
        return ip;
    }
    return QString();
}

QBitArray FCommon::bytesToBits(const QByteArray &bytes)
{
    QBitArray bits(bytes.count() * 8);

    for(int i = 0; i < bytes.count(); ++i) {
        for(int b = 0; b < 8; ++b) {
            bits.setBit(i*8 + b, bytes.at(i) & (1 << b));
        }
    }
    return bits;
}

QByteArray FCommon::bitsToBytes(const QBitArray &bits)
{
    QByteArray bytes;
    bytes.resize(bits.count()/8);

    for(int b = 0; b < bits.count(); ++b) {
        bytes[b/8] = ( bytes.at(b/8) | ((bits[b] ? 1 : 0) << (b % 8)));
    }
    return bytes;
}

