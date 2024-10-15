#ifndef F_COMMON_H
#define F_COMMON_H

#include <QObject>
#include <QByteArray>
#include <QBitArray>

class FCommon : public QObject
{
    Q_OBJECT
public:
    static FCommon *GetInstance();

    static const int Version_major{ 3 };
    static const int Version_minor{ 6 };
    static const int Version_patch{ 16 };

    static void printSystemInfo();

    static QString appName();
    static QString appVersion();
    static QString appFullVersion();
    static QString releaseDate();

    /* path */
    static QString appPath();
    static QString getPath(const QString &dirName);
    static QString checkIpAddress(const QString &address);

    static QBitArray bytesToBits(const QByteArray &bytes);
    static QByteArray bitsToBytes(const QBitArray &bits);

    void setVerbose() { m_verbose = true; }
    bool isVerbose() { return m_verbose; }

private:
    explicit FCommon(QObject *parent = nullptr);
    Q_DISABLE_COPY(FCommon)

    bool m_verbose;
};

#endif // F_COMMON_H
