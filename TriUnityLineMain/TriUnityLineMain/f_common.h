#ifndef F_COMMON_H
#define F_COMMON_H

#include <QObject>
#include <QDir>

class FCommon : public QObject
{
    Q_OBJECT
public:
    static FCommon *GetInstance();

    static const int Version_major{ 3 };
    static const int Version_minor{ 2 };
    static const int Version_patch{ 0 };

    static void printSystemInfo();

    static QString appName();
    static QString appVersion();
    static QString releaseDate();

    /* path */
    static QString appPath();
    static QString configPath();
    static QString logPath();

    static QString checkIpAddress(const QString &address);

    void setDebugMode(bool debug);
    bool isDebugMode() { return m_isDebugMode; }

private:
    explicit FCommon(QObject *parent = nullptr);
    Q_DISABLE_COPY(FCommon)

    bool m_isDebugMode;
};

#endif // F_COMMON_H
