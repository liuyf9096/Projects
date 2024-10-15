#ifndef F_FILE_MANAGER_H
#define F_FILE_MANAGER_H

#include <QByteArray>
#include <QString>
#include <QJsonObject>

class FFileManager
{
public:
    enum FileLocation { POS_APP, POS_CONFIG, POS_USER };

    static QByteArray readFile(const QString &fileName, FileLocation loc = POS_APP);
    static QJsonObject readJsonFileObj(const QString &fileName, FileLocation loc = POS_APP);
    static QJsonObject readBinaryFileObj(const QString &fileName, FileLocation loc = POS_APP);
    static bool writeJsonFileObj(const QJsonObject &obj, const QString &fileName, FileLocation loc = POS_APP);
    static bool writeFile(const QString &fileName, const QByteArray &data, FileLocation loc = POS_APP);
    static bool writeFile(const QString &fileName, const QString &text, FileLocation loc = POS_APP);
};

#endif // F_FILE_MANAGER_H
