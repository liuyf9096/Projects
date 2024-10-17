#include "DProgramDownload.h"

#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QDebug>

#ifdef Q_OS_WIN
#include <fileapi.h>
#elif defined (Q_OS_MAC)

#endif

DProgramDownload::DProgramDownload(QObject *parent) : QObject(parent)
{
    m_isDownloading = false;

    QString homeLocationPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    m_localTempDir.setPath(homeLocationPath);

    if (!m_localTempDir.exists("dobotlink-code")) {
        m_localTempDir.mkdir("dobotlink-code");
    }
    m_localTempDir.cd("dobotlink-code");
    if (!m_localTempDir.exists("phythonProject")) {
        m_localTempDir.mkdir("phythonProject");
    }
    m_localTempDir.cd("phythonProject");
}

bool DProgramDownload::searchDeviceDrive()
{
#ifdef Q_OS_WIN
    enum DRIVE_TYPE {
        UNKNOWN,            // 无法检测驱动器的类型
        ROOT_INVALID,       // 根目录无效
        DRIVE_REMOVEABLE,   // 可移动驱动器
        DRIVE_FIXED,        // 不可移动驱动器
        DRIVE_REMOTE,       // 网络驱动器
        DRIVE_CDROM,        // 光驱
        DRIVE_RAMDISK       // 虚拟驱动器
    };

    foreach (const QFileInfo &fileInfo, QDir::drives()) {
        QString driveName = fileInfo.absolutePath();
        qDebug() << "driveName" << driveName;
        LPCSTR driveName_char = reinterpret_cast<LPCSTR>(driveName.toLatin1().data());

        quint32 driveType = GetDriveTypeA(driveName_char);
        if (driveType == DRIVE_REMOVEABLE) {
            DWORD lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters;
            GetDiskFreeSpaceA(driveName_char,
                              reinterpret_cast<LPDWORD>(&lpSectorsPerCluster),
                              reinterpret_cast<LPDWORD>(&lpBytesPerSector),
                              reinterpret_cast<LPDWORD>(&lpNumberOfFreeClusters),
                              reinterpret_cast<LPDWORD>(&lpTotalNumberOfClusters));

            qDebug() << driveName << "drive type:" << driveType
                     << ", Sectors Per Cluster:" << lpSectorsPerCluster
                     << ", Bytes Per Sector:" << lpBytesPerSector
                     << ", Number Of Free Clusters:" << lpNumberOfFreeClusters
                     << ", Total Number Of Clusters:" << lpTotalNumberOfClusters;

            if (lpTotalNumberOfClusters < 500) {
                qDebug() << "Find MagicBox drive:" << driveName;

                m_boxDir.setPath(driveName);
                if (m_boxDir.exists("Script")) {
                    m_boxDir.cd("Script");
                    return true;
                } else {
                    QJsonObject errorObj;
                    errorObj.insert("code", 0);
                    errorObj.insert("message", "can not find dir Script.");
                    emit onDownloadFinished_signal(m_handleID, false, errorObj);
                }
            }
        }
    }
#elif defined (Q_OS_MAC)
    QProcess process;

    process.start("diskutil list");
    process.waitForFinished();

    QString processRes;
    processRes.prepend(process.readAllStandardOutput());

    QStringList devList = processRes.split("/dev/disk");
    for (int i=0; i<devList.size(); i++)
    {
        if (devList.at(i).contains("(external, physical):"))
        {
            QStringList devInfoList = devList.at(i).split(" ");
            for (int j=0; j<devInfoList.size(); j++)
            {
                if (!devInfoList.at(j).isEmpty())
                {
                    m_boxDir.setPath("/Volumes/" + devInfoList.at(j));
                    if (m_boxDir.exists("Script"))
                    {
                        m_boxDir.cd("Script");
                        return true;
                    }
                }
            }
            QJsonObject errorObj;
            errorObj.insert("code", 0);
            errorObj.insert("message", "can not find dir Script.");
            emit onDownloadFinished_signal(m_handleID, false, errorObj);
        }
    }
#endif

    return false;
}

bool DProgramDownload::writeToTempFile(QString text)
{
    if (m_codePyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {

        QTextStream out(&m_codePyFile);
        out << text;
        m_codePyFile.close();
        return true;
    }
    return false;
}

bool DProgramDownload::copyTempToMagicBox()
{
    if (m_boxPyFile.exists()) {
        m_boxPyFile.remove();
    }
    bool ok = QFile::copy(m_codePyFile.fileName(), m_boxPyFile.fileName());
    if (ok == false) {
        QJsonObject errorObj;
        errorObj.insert("code", 0);
        errorObj.insert("message", "copy to box failed.");
        emit onDownloadFinished_signal(m_handleID, false, errorObj);
    }
    return ok;
}

void DProgramDownload::handleProgramRequest(QString fileName, QString code, quint64 id)
{
    if (m_isDownloading == true) {
        qDebug() << "box is downloading, please try again later.";
        return;
    }

    m_handleID = id;

    if (searchDeviceDrive() == true) {
        if (!fileName.endsWith(".py")) {
            fileName.append(".py");
        }
        m_codePyFile.setFileName(m_localTempDir.absoluteFilePath(fileName));
        m_boxPyFile.setFileName(m_boxDir.absoluteFilePath(fileName));

        m_isDownloading = true;

        if (writeToTempFile(code) == true) {
            if (copyTempToMagicBox() == true) {
                emit onDownloadFinished_signal(m_handleID, true);
                qDebug() << "download request finish OK, id:" << m_handleID;
            }
        }
    }
    m_isDownloading = false;
}


