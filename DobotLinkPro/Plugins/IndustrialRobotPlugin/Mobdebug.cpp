#include "Mobdebug.h"

#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QDebug>

Mobdebug::Mobdebug(QObject *parent) : QObject(parent)
{
    m_state = MODEBUG_IDLE;
    isBusy = false;
    m_handlingId = 0;

    _processInit();
    _udpInit();
}

void Mobdebug::_processInit()
{
    QString appPath = QCoreApplication::applicationDirPath().remove("_d");
    QDir dir(appPath);

#ifdef Q_OS_MAC
    dir.cdUp();
    dir.cd("Resources");
#endif

    if (dir.exists("tool")) {
        dir.cd("tool");
#ifdef Q_OS_WIN
        dir.cd("Lua");
#elif defined (Q_OS_MAC)
        dir.cd("lua-mac");
#endif
    } else {
        qDebug() << "work space error:" << dir;
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setWorkingDirectory(dir.absolutePath());
    connect(m_process, &QProcess::started, this, &Mobdebug::onProcessStarted_slot);
    connect(m_process, &QProcess::readyRead, this, &Mobdebug::onProcessReadyRead_slot);
    connect(m_process, &QProcess::errorOccurred, this, &Mobdebug::onProcessErrorOccurred_slot);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Mobdebug::onProcessFinished_slot);
    connect(m_process, &QProcess::stateChanged, this, &Mobdebug::onProcessStateChanged);

#ifdef Q_OS_WIN
    QString program = "Lua.exe";
#elif defined (Q_OS_MAC)
    QString program = "lua";
#endif

    if (dir.exists(program)) {
        m_process->setProgram(dir.absoluteFilePath(program));
    } else {
        qDebug() << "Lua:" << program << "path is not correct.";
    }
}

void Mobdebug::_udpInit()
{
    udpPrint = new QUdpSocket(this);
    udpPrint->setProperty("id", "ClientMsg");
    connect(udpPrint, &QUdpSocket::readyRead, this, &Mobdebug::readPendingDatagrams_slot);

    udp1Cursor = new QUdpSocket(this);
    udp1Cursor->setProperty("id", "1stCursorMsg");
    connect(udp1Cursor, &QUdpSocket::readyRead, this, &Mobdebug::readPendingDatagrams_slot);

    udp2Cursor = new QUdpSocket(this);
    udp2Cursor->setProperty("id", "2ndCursorMsg");
    connect(udp2Cursor, &QUdpSocket::readyRead, this, &Mobdebug::readPendingDatagrams_slot);

    udpSpecial = new QUdpSocket(this);
    udpSpecial->setProperty("id", "SpecialMsg");
    connect(udpSpecial, &QUdpSocket::readyRead, this, &Mobdebug::readPendingDatagrams_slot);
}

bool Mobdebug::listen(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;

    QStringList arguments;
    arguments << "-e" << "require('mobdebug').listen()";

    m_process->setArguments(arguments);
    m_process->start();
    bool ok = m_process->waitForStarted();
    return ok;
}

void Mobdebug::udpOpen()
{
    if (!udpPrint->isOpen()) {
        udpPrint->bind(5000);
    }
    if (!udp1Cursor->isOpen()) {
        udp1Cursor->bind(5001);
    }
    if (!udp2Cursor->isOpen()) {
        udp2Cursor->bind(5002);
    }
    if (!udpSpecial->isOpen()) {
        udpSpecial->bind(5003);
    }
}

void Mobdebug::udpClose()
{
    udpPrint->abort();
    udp1Cursor->abort();
    udp2Cursor->abort();
    udpSpecial->abort();
}

void Mobdebug::mo_run(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("run");
}

void Mobdebug::mo_suspend(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("suspend");
}

void Mobdebug::mo_step(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("step");
}

void Mobdebug::mo_over(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("over");
}

void Mobdebug::mo_eval(QString exp, quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("eval " + exp);
}

void Mobdebug::mo_exec(QString stmt, quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("exec " + stmt);
}

void Mobdebug::mo_setb(QString file, int line, quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite(QString("setb %1 %2").arg(file).arg(line));
}

void Mobdebug::mo_delb(QString file, int line, quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite(QString("delb %1 %2").arg(file).arg(line));
}

void Mobdebug::mo_delallb(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("delallb");
}

void Mobdebug::mo_stack(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("stack");
}

void Mobdebug::mo_help(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
    _processWrite("help");
}

void Mobdebug::mo_exit(quint64 id)
{
    qDebug() << __FUNCTION__;
    m_handlingId = id;
//    _processWrite("exit");

//    if (this->m_state == MODEBUG_STOP) {
//        return;
//    }

    //断开之前的所有信号连接
//    m_process->disconnect();

    //关闭子进程
    m_process->kill();
    m_process->waitForFinished();
    qDebug() << "kill server, MobDebugStop";

    //抛出状态改变信号
    this->m_state = MODEBUG_STOP;
    emit onModebugStateChanged_signal(m_state, m_handlingId);
    emit onFinish_signal(m_handlingId);
}

bool Mobdebug::_processWrite(QString cmd)
{
    if (m_process == nullptr) {
        return false;
    }

//    if (isBusy == true) {
//        qDebug() << "process is buzy, please try again later.";
//        return false;
//    }
    if (m_process->state() == QProcess::Running) {
        isBusy = true;
        m_process->write((cmd + "\r\n").toUtf8());
        return true;
    }
    qDebug() << "process is not running.";
    return false;
}

//![process slot]
void Mobdebug::onProcessStarted_slot()
{
    isBusy = true;
}

void Mobdebug::onProcessReadyRead_slot()
{
    QString readStr = QString::fromLatin1(m_process->readAll());
    emit onModebugMessage_signal(readStr);
    qDebug() << ">>[process]" << readStr;

    if (readStr.contains("Server started")) {
        m_state = MODEBUG_STARTED;
        emit onModebugStateChanged_signal(m_state, m_handlingId);
    }
    if (readStr.contains("Waitting client")) {
        m_state = MODEBUG_LISTENING;
        emit onModebugStateChanged_signal(m_state, m_handlingId);
    }
    if (readStr.contains("Client connected")) {
        m_state = MODEBUG_CONNECTED;
        isBusy = false;
        emit onModebugStateChanged_signal(m_state, m_handlingId);
        emit onFinish_signal(m_handlingId);
    }
    if (readStr.contains("Comm: run")) {
        m_state = MODEBUG_RUNNING;
        isBusy = false;
        emit onModebugStateChanged_signal(m_state, m_handlingId);
        emit onFinish_signal(m_handlingId);
    }
    if (readStr.contains("Server closed")) {
        m_state = MODEBUG_STOP;
        isBusy = false;
        emit onModebugStateChanged_signal(m_state, m_handlingId);
        emit onFinish_signal(m_handlingId);
    }
}

void Mobdebug::onProcessErrorOccurred_slot(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << error;
}

void Mobdebug::onProcessFinished_slot(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << Q_FUNC_INFO << exitCode << exitStatus;
}

void Mobdebug::onProcessStateChanged(QProcess::ProcessState newState)
{
    qDebug() << __FUNCTION__ << newState;
}

//![udp slot]
void Mobdebug::readPendingDatagrams_slot()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket*>(sender());
    if (socket) {
        qDebug() << "receive:" << socket->property("id").toString();

        while (socket->hasPendingDatagrams()) {
            QNetworkDatagram datagram = socket->receiveDatagram();
            QString message = datagram.data();
            qDebug() << "[udp]" << sender()->objectName() << message;
        }
    }
}
