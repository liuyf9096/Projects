#ifndef F_FILE_MANAGER_H
#define F_FILE_MANAGER_H

#include <QByteArray>
#include <QString>

class FFileManager
{
public:
    enum FileLocation { POS_APP, POS_USER };

    static QByteArray readFile(const QString &fileName, FileLocation loc = POS_APP);
    static bool writeFile(const QString &fileName, const QByteArray &data, FileLocation loc = POS_APP);
    static bool writeFile(const QString &fileName, const QString &text, FileLocation loc = POS_APP);
};

#endif // F_FILE_MANAGER_H
