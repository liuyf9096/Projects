#include "m_config_task.h"
#include <QProcess>

MConfigTask::MConfigTask(QObject *parent)
    : MProcessBase{"synsystemtime", "synsystemtime", parent}
{
    _init();
}

void MConfigTask::_init()
{

}

void MConfigTask::state_init()
{
    _init();
}

void MConfigTask::setSystemTime(quint64 id, const QJsonObject &obj)
{
    m_id = id;

    int year = obj.value("year").toInt();
    int month = obj.value("month").toInt();
    int day = obj.value("day").toInt();
    int hh = obj.value("hh").toInt();
    int mm = obj.value("mm").toInt();
    int sec = obj.value("sec").toInt();
    int msec = obj.value("msec").toInt();

    setSystemTime(year, month, day, hh, mm, sec, msec);

    mTimer->start();
}

void MConfigTask::setSystemTime(int year, int month, int day, int hh, int mm, int sec, int msec)
{
    QString dt;
    dt.sprintf("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hh, mm, sec);

    QProcess systime;
    systime.start("date", {"-s", dt});
    systime.waitForFinished();
    systime.start("hwclock", {"-w"});
    systime.waitForFinished();
    systime.start("sync", {""});
    systime.waitForFinished();

    if (dev->track()->isEnable()) {
        dev->track()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);
    }
    dev->smear()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);
    dev->stain()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);
    dev->sample()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);

#if 0
    dev->print()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);
    dev->ultrasound()->cmd_SystemTimeSync(year, month, day, hh, mm, sec, msec);
#endif
}

void MConfigTask::onTimer_slot()
{
    QMap<QString, bool> map;

    if (dev->track()->isEnable()) {
        bool ok = dev->track()->isFuncDone("SystemTimeSync");
        map.insert("track", ok);
    }
    {
        bool ok = dev->smear()->isFuncDone("SystemTimeSync");
        map.insert("smear", ok);
    }
    {
        bool ok = dev->stain()->isFuncDone("SystemTimeSync");
        map.insert("stain", ok);
    }
    {
        bool ok = dev->sample()->isFuncDone("SystemTimeSync");
        map.insert("sample", ok);
    }

    bool done = true;
    foreach (auto ok, map.values()) {
        if (ok == false) {
            done = false;
        }
    }
    if (done == true) {
        JPacket p(PacketType::Result, m_id);
        p.resValue = true;
        sendUIMessage(p);

        mTimer->stop();
    }
}

