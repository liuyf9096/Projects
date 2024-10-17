#include "DownloadPlugin.h"

#include <QDebug>
#include "MessageCenter/DPacket.h"

const QString DownloadPlugin::PluginName = "Download";
const QString DownloadPlugin::Version = "1.0.0";

DownloadPlugin::DownloadPlugin(QObject *parent) : DPluginInterface(parent)
{
    m_dfufile = new DDfufile(this);
    connect(m_dfufile, &DDfufile::onProcessReadyRead_signal, this, [=](QString message){
        DNotificationPacket packet(m_handlingWsPort);
        QJsonObject messageObj;
        messageObj.insert("message", message);
        QJsonObject noticeObj = packet.getNotificationObj("dobotLink.dfudownload.process", messageObj);
        emit pSendMessage_signal(PluginName, noticeObj);
    });
    connect(m_dfufile, &DDfufile::onFinish_signal,
            this, &DownloadPlugin::onFinish_slot);
}

QString DownloadPlugin::getVersion()
{
    return Version;
}

/* 收到消息 */
void DownloadPlugin::pReceiveMassage_slot(QString id, QJsonObject obj)
{
    if (id == "ALL") {
        qDebug() << "[ALL] {Download} get obj" << obj;
        if (obj.contains("METHOD")) {
            QString method = obj.value("METHOD").toString();
            if (method == "EXIT") {
                qDebug() << "DownloadPlugin will EXIT.";
            }
        }
    } else if (id == PluginName) {
        handleDfuDownloadCmd(obj);
    }
}

void DownloadPlugin::onFinish_slot(bool ok)
{
    qDebug() << "dfu finish:" << ok;

    DRequestPacket packet = m_requestPacketMap.value(m_handlingId);
    DResultPacket resPacket(packet);
    if (ok) {
        emit pSendMessage_signal(PluginName, resPacket.getResultObj());
    } else {
        QJsonObject errorObj;
        errorObj.insert("code", 300);
        errorObj.insert("message", "Dfu download failed.");
        emit pSendMessage_signal(PluginName, resPacket.setErrorObj(errorObj));
    }
}


/* [!!!消息分发!!!] */
void DownloadPlugin::handleDfuDownloadCmd(const QJsonObject &obj)
{
    DRequestPacket packet;
    packet.setPacketFromObj(obj);

    m_requestPacketMap.insert(packet.id, packet);
    m_handlingId = packet.id;
    m_handlingWsPort = packet.wsPort;

    qDebug() << "handleDfuDownloadCmd" << obj;

    if (packet.api == "firmware") {
        QString device = packet.getParamValue("device").toString();
        if (device == "MagicBox") {
            m_dfufile->m_type = DDfufile::DFU_MAGICBOX;
        } else if (device == "MagicianLite") {
            m_dfufile->m_type = DDfufile::DFU_MAGICIANLITE;
        } else {
            qDebug() << "please specify device type.";
            return;
        }

        QString fileName = packet.getParamValue("fileName").toString();
        m_dfufile->setFileName(fileName);

        m_dfufile->startDownload();
    }

}


