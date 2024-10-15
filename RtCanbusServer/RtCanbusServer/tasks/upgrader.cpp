#include "upgrader.h"
#include "sql/f_sql_database_manager.h"
#include "f_common.h"
#include "messagecenter/f_jsonrpc_parser.h"
#include "device/rt_device.h"
#include "device/device_protocol.h"
#include "device/rt_device_manager.h"

#include <QJsonDocument>
#include <QDir>
#include <QTimer>
#include <QDebug>

Upgrader *Upgrader::GetInstance()
{
    static Upgrader *instance = nullptr;
    if (instance == nullptr) {
        instance = new Upgrader();
    }
    return instance;
}

Upgrader::Upgrader(QObject *parent)
    : QObject{parent}
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10);
    connect(m_timer, &QTimer::timeout, this, &Upgrader::onTimerTimeout_slot);

    s_state = Process::Idle;

    mDB = FSqlDatabaseManager::GetInstance()->getDatebase("upgrade");
}

/* fileName : project.json / test.project */
void Upgrader::upgradeBoards(QString fileName)
{
    qDebug() << "upgrade board, filename:" << fileName;

    if (mDB) {
        if (!fileName.contains("/")) {
            QDir dir(FCommon::getPath("upgrade"));
            if (dir.exists(fileName)) {
                fileName = dir.absoluteFilePath(fileName);
            }
        }

        QJsonObject obj = readUpgradeFile(fileName);
        if (!obj.isEmpty()) {
            parseContent(obj);
        }
    }
}

QJsonObject Upgrader::readUpgradeFile(const QString &fileName)
{
    qDebug() << "read upgrade file:" << fileName;

    QJsonObject obj;
    QFile file(fileName);
    if (fileName.endsWith("json")) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &jsonError);
            if (jsonError.error == QJsonParseError::NoError) {
                obj = doc.object();
            }
        }
    } else if (fileName.endsWith("project")) {
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromBinaryData(file.readAll());
            obj = doc.object();
        }
    }
    return obj;
}

void Upgrader::parseContent(const QJsonObject &obj)
{
    QString project_name = obj.value("project_name").toString();
    qDebug() << "handle project, name:" << project_name;

    m_boardArray = obj.value("board").toArray();
    if (!m_boardArray.isEmpty()) {
        m_timer->start();
    }
}

bool Upgrader::parseUpgradeObj(const QJsonObject &obj)
{
    int board_id = obj.value("board_id").toInt();
    device_id = QString("board_%1").arg(board_id);
    qDebug().noquote() << QString("board_id: 0x%1").arg(board_id, 0, 16);

    m_device = RtDeviceManager::GetInstance()->getDevice(board_id);
    if (m_device == nullptr) {
        qDebug() << "can NOT find device:, address:" << board_id;
        return false;
    }

    QString board_name = obj.value("board_name").toString();
    qDebug() << "board_name:" << board_name;

    device_type = obj.value("board_type").toString();
    qDebug() << "board_type:" << device_type;

    QString datetime = obj.value("datetime").toString();
    qDebug() << "datetime:" << datetime;

    m_version = obj.value("version").toString();
    qDebug() << "version:" << m_version << endl;

    combo_programArr = obj.value("combo_program").toArray();
    return true;
}

QByteArray Upgrader::handleFunction(const QString &board_type, const QString &board_id, quint16 line, const QString &funcBody)
{
    QStringList list = funcBody.split(",", QString::SkipEmptyParts);
    qDebug().noquote() << QString(" line:%1 (0x%2)").arg(line).arg(line, 4, 16, QLatin1Char('0'));

    QByteArray byteArr;
    byteArr.append(reinterpret_cast<char*>(&line), sizeof (quint16));

    for (int i = 0; i < list.count(); ++i) {
        QString character = list.at(i);
        if (i == 0) {
            byteArr.append(handleFunctionName(board_type, character));
        } else {
            byteArr.append(handleParams(board_id, character));
        }
    }
    return byteArr;
}

QByteArray Upgrader::handleFunctionName(const QString &board_type, const QString &funcName)
{
    QByteArray data;
    if (funcName.isEmpty()) {
        return data;
    }

    QString fname = QString("'%1'").arg(funcName);
    QJsonArray arr = mDB->selectRecord(board_type, QString("keyword='%1'").arg(funcName));
    if (arr.isEmpty() == false)
    {
        QJsonObject obj = arr.first().toObject();
        QJsonValue value = obj.value("value");
        if (value.isDouble()) {
            quint16 v_i = static_cast<quint16>(value.toDouble());

            qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(fname, 30).arg(v_i).arg(v_i, 4, 16, QLatin1Char('0'));

            data.append(reinterpret_cast<char*>(&v_i), sizeof (quint16));
            return data;
        } else if (value.isString()) {
            QString v = value.toString();
            bool ok = false;
            quint16 v_i = v.toInt(&ok, 16);
            if (ok) {
                qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(fname, 30).arg(v_i).arg(v_i, 4, 16, QLatin1Char('0'));

                QByteArray data;
                data.append(reinterpret_cast<char*>(&v_i), sizeof (quint16));
                return data;
            }
        }
    } else {
        QJsonArray arr1 = mDB->selectRecord(board_type, QString("username='%1'").arg(funcName));
        if (arr1.isEmpty() == false)
        {
            QJsonObject obj = arr1.first().toObject();
            QJsonValue value = obj.value("value");
            if (value.isDouble()) {
                quint16 v_i = static_cast<quint16>(value.toDouble());
                qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(fname, 30).arg(v_i).arg(v_i, 4, 16, QLatin1Char('0'));

                data.append(reinterpret_cast<char*>(&v_i), sizeof (quint16));
                return data;
            } else if (value.isString()) {
                QString v = value.toString();
                bool ok = false;
                quint16 v_i = v.toInt(&ok, 16);
                if (ok) {
                    qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(fname, 30).arg(v_i).arg(v_i, 4, 16, QLatin1Char('0'));

                    QByteArray data;
                    data.append(reinterpret_cast<char*>(&v_i), sizeof (quint16));
                    return data;
                }
            }
        } else {
            qDebug() << "handleFunctionName Error!" << funcName;
        }
    }
    return data;
}

QByteArray Upgrader::handleParams(const QString &board_id, const QString &params)
{
    QByteArray data;
    if (params.isEmpty()) {
        return data;
    }


    bool isNum = false;
    quint32 param_i = params.toUInt(&isNum);
    if (isNum) {
        QString pname = QString("'%1'").arg(param_i);
        qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(pname, 30).arg(param_i).arg(param_i, 8, 16, QLatin1Char('0'));
        data.append(reinterpret_cast<char*>(&param_i), sizeof (quint32));
        return data;
    }

    QJsonArray arr = mDB->selectRecord(board_id, QString("keyword='%1'").arg(params));
    if (arr.isEmpty() == false) {
        QString pname = QString("'%1'").arg(params);
        QJsonObject obj = arr.first().toObject();
        QJsonValue value = obj.value("value");
        if (value.isDouble()) {
            quint32 v_i = static_cast<quint32>(value.toDouble());
            qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(pname, 30).arg(v_i).arg(v_i, 8, 16, QLatin1Char('0'));

            data.append(reinterpret_cast<char*>(&v_i), sizeof (quint32));
            return data;
        } else if (value.isString()) {
            QString v = value.toString();
            bool ok = false;
            quint32 v_i = v.toInt(&ok, 16);
            if (ok) {
                qDebug().noquote() << QString(" %1 => %2 (0x%3)").arg(pname, 30).arg(v_i).arg(v_i, 8, 16, QLatin1Char('0'));

                QByteArray data;
                data.append(reinterpret_cast<char*>(&v_i), sizeof (quint32));
                return data;
            }
        }
    } else {
        qDebug() << "handleParams Error!" << params;
    }
    return data;
}

QVector<quint8> Upgrader::getVersion(const QString &version)
{
    QStringList list = version.split(".");
    QVector<quint8> v_arr;
    if (list.count() > 3) {
        for (int i = 0; i < 4; ++i) {
            QString v = list.at(i);
            quint8 v_i = v.toUInt();
            v_arr.append(v_i);
        }
    }
    return v_arr;
}

void Upgrader::onTimerTimeout_slot()
{
    switch (s_state) {
    case Process::Idle:
        s_state = Process::Prepare_Board;
        break;

    case Process::Prepare_Board:
        if (!m_boardArray.isEmpty()) {
            m_boardObj = m_boardArray.takeAt(0).toObject();
            bool ok = parseUpgradeObj(m_boardObj);
            if (ok) {
                s_state = Process::ComboDownload_Start;
            }
        } else {
            s_state = Process::Finish;
        }
        break;

    case Process::ComboDownload_Start:
        if (combo_programArr.isEmpty() == false) {
            QVector<quint8> version = getVersion(m_version);
            quint16 count = combo_programArr.count();

            qDebug().noquote() << QString("1.Downloading programs...  board:%1 version:%2 count:%3")
                                  .arg(device_id).arg(m_version).arg(count);

            cmd_ComboActionDownload(version, count);
            s_state = Process::WaitF_1_Done;
        } else {
            qDebug() << "666";
        }
        break;
    case Process::WaitF_1_Done:
        if (m_device->isSelfFuncDone("ComboActionDownload")) {
            combo_program_index = 0;
            s_state = Process::Combo_Head_Start;
        }
        break;

    case Process::Combo_Head_Start:
        if (combo_program_index < combo_programArr.count()) {
            combo_programObj = combo_programArr.at(combo_program_index).toObject();

            f_func_id = combo_programObj.value("func_id").toInt();
            f_program = combo_programObj.value("program").toArray();
            f_count = f_program.count();
            f_decription = combo_programObj.value("decription").toString();

            qDebug().noquote() << QString("2.PARSE function:%1 %2 count:%3").arg(f_func_id).arg(f_decription).arg(f_count);

            cmd_ComboActionHead(f_func_id, f_count);
            s_state = Process::WaitF_2_Done;
        }
        break;
    case Process::WaitF_2_Done:
        if (m_device->isSelfFuncDone("ComboActionHead")) {
            f_line = 0;
            s_state = Process::Combo_Body_Start;
        }
        break;

    case Process::Combo_Body_Start:
        if (f_line < f_program.count()) {
            QString func = f_program.at(f_line).toString();
            qDebug().noquote() << QString("3.PARSE line:%1 %2").arg(f_line).arg(func);

            QByteArray data = handleFunction(device_type, device_id, f_line, func);
            if (data.isEmpty() == false) {
                m_device->RunFunctionArg("ComboActionBody", data);
            }
            s_state = Process::WaitF_3_Done;
        } else {
            s_state = Process::Combo_End_Start;
        }
        break;
    case Process::WaitF_3_Done:
        if (m_device->isSelfFuncDone("ComboActionBody")) {
            f_line++;
            s_state = Process::Combo_Body_Start;
        }
        break;

    case Process::Combo_End_Start:
        qDebug().noquote() << QString("4.PARSE function Finish. func_id:%1 %2")
                              .arg(f_func_id).arg(f_decription);

        cmd_ComboActionEnd(f_func_id, f_count);
        s_state = Process::WaitF_4_Done;
        break;
    case Process::WaitF_4_Done:
        if (m_device->isSelfFuncDone("ComboActionEnd")) {
            s_state = Process::ComboDownload_End;
        }
        break;

    case Process::ComboDownload_End:
        {
            QVector<quint8> version = getVersion(m_version);
            quint16 count = combo_programArr.count();

            qDebug().noquote() << QString("5.Download Finish.  board:%1 version:%2 count:%3")
                                  .arg(device_id).arg(m_version).arg(count);

            cmd_ComboActionDownloadFinish(version, count);
            s_state = Process::WaitF_5_Done;
        }
        break;
    case Process::WaitF_5_Done:
        if (m_device->isSelfFuncDone("ComboActionDownloadFinish")) {
            s_state = Process::Prepare_Board;
        }
        break;

    case Process::Finish:
        qDebug() << "99.Upgrade Finish.";
        m_timer->stop();
        s_state = Process::Idle;
        break;
    }
}

void Upgrader::cmd_ComboActionDownload(QVector<quint8> version, quint16 count)
{
    if (m_device) {
        QByteArray data = DeviceProtocol::ComboActionDownload(version, count);
        m_device->RunFunctionArg("ComboActionDownload", data);
    }
}

void Upgrader::cmd_ComboActionHead(quint16 func_id, quint16 count)
{
    if (m_device) {
        QByteArray data = DeviceProtocol::ComboActionHead(func_id, count);
        m_device->RunFunctionArg("ComboActionHead", data);
    }
}

void Upgrader::cmd_ComboActionBody(quint16 index, quint16 func_id, QVector<quint32> params)
{
    if (m_device) {
        QByteArray data = DeviceProtocol::ComboActionBody(index, func_id, params);
        m_device->RunFunctionArg("ComboActionHead", data);
    }
}

void Upgrader::cmd_ComboActionEnd(quint16 func_id, quint16 count)
{
    if (m_device) {
        QByteArray data = DeviceProtocol::ComboActionEnd(func_id, count);
        m_device->RunFunctionArg("ComboActionEnd", data);
    }
}

void Upgrader::cmd_ComboActionDownloadFinish(QVector<quint8> version, quint16 count)
{
    if (m_device) {
        QByteArray data = DeviceProtocol::ComboActionDownloadFinish(version, count);
        m_device->RunFunctionArg("ComboActionDownloadFinish", data);
    }
}

void Upgrader::setBoardProgram(const QJsonObject &obj)
{
    int board_id = obj.value("board_id").toInt();
    qDebug().noquote() << QString("board_id: 0x%1").arg(board_id, 0, 16);

    QString board_name = obj.value("board_name").toString();
    qDebug() << "board_name:" << board_name;

    QString board_type = obj.value("board_type").toString();
    qDebug() << "board_type:" << board_type;

    QString datetime = obj.value("datetime").toString();
    qDebug() << "datetime:" << datetime;

    QString version = obj.value("version").toString();
    qDebug() << "version:" << version << endl;

    QJsonArray combo_program = obj.value("combo_program").toArray();
    for (int i = 0; i < combo_program.count(); ++i) {
        QJsonObject obj = combo_program.at(i).toObject();
        handleSequence(board_type, QString("board_%1").arg(board_id), obj);
    }
}

void Upgrader::handleSequence(const QString &board_type, const QString &board_id, const QJsonObject &obj)
{
    qDebug() << "Sequence board_type:" << board_type << "board_id:" << board_id;

    int program_num = obj.value("program_num").toInt();
    QString program_decription = obj.value("program_decription").toString();

    qDebug().noquote() << QString("PARSE function:%1 %2").arg(program_num).arg(program_decription);

    QJsonArray program = obj.value("program").toArray();
    for (quint16 i = 0; i < program.count(); ++i) {
        QString func = program.at(i).toString();
        handleFunction(board_type, board_id, i, func);
    }
    qDebug().noquote() << QString("PARSE function:%1 END").arg(program_num) << endl;
}
