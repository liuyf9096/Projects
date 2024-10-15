#include "task_handler.h"

#include "file/f_file_manager.h"

static const QString DevID = "fileHandler";

TaskHandler *TaskHandler::GetInstance()
{
    static TaskHandler *instance = nullptr;
    if (instance == nullptr) {
        instance = new TaskHandler();
    }
    return instance;
}

TaskHandler::TaskHandler(QObject *parent) : QObject(parent)
{
#if 0
    auto ws = FMessageCenter::GetInstance();
    connect(ws, &FMessageCenter::onReceivePacket_signal,
            this, &TaskHandler::handleReceivePacket_slot);
#endif
}

void TaskHandler::handleReceivePacket_slot(const QString &dev_id, const JPacket &p)
{
    if (dev_id != DevID) {
        return;
    }

    if (p.api == "writeFile") {
        QString fileName = p.paramsValue.toObject().value("fileName").toString();
        QString content = p.paramsValue.toObject().value("content").toString();

        FFileManager::writeFile(fileName, content);
    } else if (p.api == "readFile") {
        QString fileName = p.paramsValue.toObject().value("fileName").toString();
        QByteArray data = FFileManager::readFile(fileName);

        JPacket result(PacketType::Result, p.id);
        result.resValue = QString(data);

//        FMessageCenter::GetInstance()->sendMessage(result, DevID);
    }
}
