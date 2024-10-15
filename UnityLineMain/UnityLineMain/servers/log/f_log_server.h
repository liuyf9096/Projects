#ifndef F_LOG_SERVER_H
#define F_LOG_SERVER_H

/***************************************************
 * function : used for record log debug message    *
 *   author : liuyufei                             *
 * datetime : 2021-04-02 15:33:39                  *
 *  version : 1.0                                  *
 ***************************************************/

#include <QObject>
#include <QJsonObject>
#include <QDir>

class FLogServerPrivate;
class FLogServer : public QObject
{
    Q_OBJECT
public:
    enum MessageFileFlags {
        Content = 0x01,
        Type = 0x02,
        Time = 0x04,
        TimeTypeContent = Time | Type | Content, /* default */
        TypeContent = Type | Content,
        File = 0x08,
        Line = 0x10,
        Funtion = 0x20,
        Category = 0x40,
        AllMessage = 0x7F
    };

    static FLogServer *GetInstance();

    void start();
    void stop();

    void setMessageFlags(MessageFileFlags flags);
    MessageFileFlags MessageFlags();
    QString getLogFilePath();

    void setMaxLogFileCount(uint count);
    QString saveToTxtEnable(bool en, quint64 index = 0);
    void setLogFileTitle(const QString &title); // default: 'RT' eg:title_yyyymmdd.txt
    void setLogFilePath(const QDir &dir);       // obsolete default: AppDataLocation: 'C:/Users/<USER>/AppData/Roaming/<ORGAN>/<APP>'

    void openLogFile();
    bool openLogDirectory();

signals:
    /* logMessage_signal's construction:
        type_int    (int)       :   0~5
        type_int    (string)    :   debug/warning/critical/fatal/info
        message     (string)    :   content
        time        (string)    :   yyyy-MM-dd hh:mm:ss.zzz
        context     (Object)    :   file, line, function, category
    */
    void logMessage_signal(const QJsonObject &obj);    // used for other record, FORBID print anymore !!! caution:reentry

private:
    explicit FLogServer(QObject *parent = nullptr);
    Q_DISABLE_COPY(FLogServer)

    FLogServerPrivate * const Dptr;
    Q_DECLARE_PRIVATE_D(Dptr, FLogServer)
};

#endif // F_LOG_SERVER_H
