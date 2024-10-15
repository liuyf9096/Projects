#ifndef UPGRADER_H
#define UPGRADER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class QTimer;
class RtDevice;
class FSqlDatabase;
class Upgrader : public QObject
{
    Q_OBJECT
public:
    static Upgrader *GetInstance();

    void upgradeBoards(QString fileName = "program.json");
    QJsonObject readUpgradeFile(const QString &fileName);
    bool parseUpgradeObj(const QJsonObject &obj);

    void setDevice(RtDevice *device) { m_device = device; }
    void cmd_ComboActionDownload(QVector<quint8> version, quint16 count);
    void cmd_ComboActionHead(quint16 func_id, quint16 count);
    void cmd_ComboActionBody(quint16 index, quint16 func_id, QVector<quint32> params);
    void cmd_ComboActionEnd(quint16 func_id, quint16 count);
    void cmd_ComboActionDownloadFinish(QVector<quint8> version, quint16 count);

private:
    explicit Upgrader(QObject *parent = nullptr);
    Q_DISABLE_COPY(Upgrader)

    FSqlDatabase *mDB = nullptr;

    QJsonArray m_boardArray;

    void parseContent(const QJsonObject &obj);
    void setBoardProgram(const QJsonObject &obj);
    void handleSequence(const QString &board_type, const QString &board_id, const QJsonObject &obj);
    QByteArray handleFunction(const QString &board_type, const QString &board_id, quint16 line, const QString &funcBody);
    QByteArray handleFunctionName(const QString &board_type, const QString &funcName);
    QByteArray handleParams(const QString &board_id, const QString &params);

    QVector<quint8> getVersion(const QString &version);

    QString m_version;
    RtDevice *m_device;
    QString device_id;
    QString device_type;

    /* function */
    quint16 f_func_id;
    quint16 f_count;
    QString f_decription;
    QJsonArray f_program;
    quint16 f_line;

    QJsonArray combo_programArr;
    QJsonObject combo_programObj;
    int combo_program_index;

    QTimer *m_timer;

    QJsonObject m_boardObj;
    enum class Process {
        Idle,

        Prepare_Board,

        ComboDownload_Start,
        WaitF_1_Done,

        Combo_Head_Start,
        WaitF_2_Done,

        Combo_Body_Start,
        WaitF_3_Done,

        Combo_End_Start,
        WaitF_4_Done,

        ComboDownload_End,
        WaitF_5_Done,

        Finish
    } s_state;

private slots:
    void onTimerTimeout_slot();
};

#endif // UPGRADER_H
