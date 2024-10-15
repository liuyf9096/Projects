#ifndef F_LOG_SERVER_P_H
#define F_LOG_SERVER_P_H

#include <QObject>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QMessageLogger>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message);

class QThread;
class FLogServer;
class FLogServerPrivate : public QObject
{
    Q_OBJECT
public:
    FLogServerPrivate(FLogServer *parent);
    virtual ~FLogServerPrivate();

private:
    FLogServer * const q_ptr;
    Q_DECLARE_PUBLIC(FLogServer)

    int m_flags;
    bool m_isLogging;
    int m_type;

    QFile mLogFile;
    QString m_logFileTitle;
    QThread *mThread;

    void creatLogFile();
    void saveMsgToFile(const QString &type, QJsonObject context, const QString &message, const QString &time);

private slots:
    void handleOriginalMessage_slot(QJsonObject obj);
};

#endif // F_LOG_SERVER_P_H
