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

    const static int Version_major = 1;
    const static int Version_minor = 2;
    const static int Version_patch = 3;

    static QString printSystemInfo();

    static QString appName();
    static QString appVersion();
    static QString releaseDate();

    /* path */
    static QString appPath();
    static QString configPath();
    static QString logPath();

    static QString checkIpAddress(const QString &address);

private:
    explicit FCommon(QObject *parent = nullptr);
    Q_DISABLE_COPY(FCommon)
};

#endif // F_COMMON_H
