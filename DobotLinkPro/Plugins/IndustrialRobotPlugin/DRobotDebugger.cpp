#include "DRobotDebugger.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkProxyFactory>

#define SLEEPTIME1 200
#define SLEEPTIME2 500

DRobotDebugger::DRobotDebugger(QObject *parent) : QObject(parent)
{
    m_state = MobDebugStop;
    m_isStepping = false;
    m_isStepFinished = false;
    m_hasError = false;

    udpPrint = new QUdpSocket(this);
//    connect(udpPrint, &QUdpSocket::readyRead, )onClientMsg
    udp1Cursor = new QUdpSocket(this);
    //    connect(udpPrint, &QUdpSocket::readyRead, )on1stCursorMsg
    udp2Cursor = new QUdpSocket(this);
    //    connect(udpPrint, &QUdpSocket::readyRead, )on2ndCursorMsg
    udpSpecial = new QUdpSocket(this);
//    connect(udpPrint, &QUdpSocket::readyRead, )onSpecialMsg

#ifdef Q_OS_WIN
        //防止端口无法绑定
        QNetworkProxyFactory::setUseSystemConfiguration(false);
#endif

    udpPrint->bind(5000);
    udp1Cursor->bind(5001);
    udp2Cursor->bind(5002);
    udpSpecial->bind(5003);

    m_process = new QProcess(this);
}

DRobotDebugger::~DRobotDebugger()
{

}

void DRobotDebugger::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime ) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
}

DRobotDebugger::DebugState DRobotDebugger::debugState(void)
{
    return this->m_state;
}

void DRobotDebugger::start(const QString &fileName)
{
    QEventLoop eventLoop;

    //设置调试状态
    if(this->m_state != MobDebugStarting) {
        this->m_state = MobDebugStarting;
        emit debugStateChanged(this->m_state);
    }

    //Stop按钮禁止使能
    emit stopStateEnabled(false);

    //进程启动成功标志
    bool result = true;
    QProcess::ProcessError errorIndex;

    //MobDebug进程正常启动
    connect(m_process, &QProcess::started, [&](){
        qDebug() << "MobDebug started!";
        result = true;
        eventLoop.quit();
    });

    //MobDebug进程启动失败
    connect(m_process, &QProcess::errorOccurred, [&](QProcess::ProcessError error) {
        qDebug() << "MobDebug errorOccurred!";
        result = false;
        errorIndex = error;
        eventLoop.quit();
    });

    QTimer::singleShot(0, [=](){
        qDebug() << "MobDebug starting!";
        QStringList arguments;
        arguments << QString("-e");
        arguments << QString("require('mobdebug').listen()");
        m_process->setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
        m_process->start(qApp->applicationDirPath() + "/Lua/Lua.exe", arguments);
#else
        m_process->start(QString("/dobot/bin/bin/lua"), arguments);
#endif
        //d->server.waitForStarted();
    });

    //等待进程启动
    eventLoop.exec();

    //如果MobDebug没有正常启动就退出
    if (!result) {
        switch (errorIndex) {
        case 0:
            qDebug() << "MobDebug Start Error: FailedToStart";
            emit mobDebugInfo("MobDebug Start Error: FailedToStart");
            break;
        case 1:
            qDebug() << "MobDebug Start Error: Crashed";
            break;
        case 2:
            qDebug() << "MobDebug Start Error: Timedout";
            break;
        case 3:
            qDebug() << "MobDebug Start Error: ReadError";
            break;
        case 4:
            qDebug() << "MobDebug Start Error: WriteError";
            break;
        case 5:
            qDebug() << "MobDebug Start Error: UnknownError";
            break;
        default:
            qDebug() << "MobDebug Start Error";
            break;
        }
        return;
    }

    static bool isStarted = false;

    //MobDebug进程和控制器的启动交互数据
    connect(m_process, &QProcess::readyRead, [=](){
        static const QString KEY_WORD(">");

        QString string = m_process->readAll();
        string = string.trimmed();
        emit mobDebugInfo("*Server(Start)*\n" + string);

        //成功启动
        if(string.contains(KEY_WORD)) {
            //断开之前的所有信号连接
            qDebug() << "server disconnect!";
            m_process->disconnect();
            //发出启动标志位
            if(this->m_state != MobDebugStarted) {
                //避免过快点击start和step
                sleep(SLEEPTIME2);
                this->m_state = MobDebugStarted;
                emit debugStateChanged(this->m_state);
                isStarted = true;
            }
        }
    });
    static bool isFirst = true;
    //第一次启动时较慢，所以不等待Started状态
    if (!isFirst) {
        //特殊处理，避免导致监听后还未绑定客户端成功，过快通过HTTP发送start指令
        m_timeout.restart();
        while (!isStarted) {
            if (m_timeout.elapsed() > 6000) {
                qDebug() << "Server started timeout!!!";
                break;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        }
        m_timeout.restart();
        isStarted = false;
    }
    isFirst = false;

    sleep(SLEEPTIME2);
    QString url = QString("/debugger/%1").arg("start");
    QJsonObject wData, rData;
    wData.insert("project", fileName);
//    d->httpManager->post(url, wData, rData);

    //避免启动过程中，切回手动模式
    sleep(SLEEPTIME2);
    if (this->m_state != MobDebugStop) {
        //Stop按钮恢复使能
        emit stopStateEnabled(true);
    }
}

void DRobotDebugger::stop(void)
{
    QString url = QString("/debugger/%1").arg("stop");
    QJsonObject wData, rData;
//    d->httpManager->post(url, wData, rData);

    this->stopMobDebug();
}

void DRobotDebugger::runScript(void)
{
    emit stopStateEnabled(false);

    QString url = QString("/debugger/%1").arg("run");
    QJsonObject wData, rData;
//    d->httpManager->post(url, wData, rData);

    //标志位为运行状态
    if (this->m_state != SrcRunning) {
        this->m_state = SrcRunning;
        emit debugStateChanged(this->m_state);
    }

    //屏蔽按钮
    emit debugStateEnabled(false);

    //MD的Run指令要放在HTTP的Run指令之后
    //不然会卡在运动指令之中
    this->SendCmd("run\r\n");

    //避免过快操作
    sleep(SLEEPTIME2);

    //如果有语法错误，则状态不恢复
    if (m_hasError) {
        m_hasError = false;
        return;
    }

    //如果出现报警，则状态不恢复
    if (this->m_state == MobDebugStop) {
        return;
    }

    emit debugStateEnabled(true);
    emit stopStateEnabled(true);
}

void DRobotDebugger::suspend(void)
{
    this->SendCmd("suspend\r\n");

    //TODO: 暂时用法，否则不能正常工作
    //先同步了TCP返回的脚本停止状态，再HTTP控制队列停止
    connect(this, &DRobotDebugger::debugStateChanged, [=](DebugState state){
        if (state == SrcSuspend) {
            QString url = QString("/debugger/%1").arg("suspend");
            QJsonObject wData, rData;
//            d->httpManager->post(url, wData, rData);
            disconnect(this, &DRobotDebugger::debugStateChanged, this, 0);
        }
    });

    //禁止运行暂停的按钮
    emit debugStateEnabled(false);

    //标志位为暂停状态
    if (this->m_state != SrcSuspend || this->m_isStepping) {
        if (this->m_isStepping) {
            this->m_isStepping = false;
        }
        this->m_state = SrcSuspend;
        emit debugStateChanged(this->m_state);
    }

    //查询控制器是否确认暂停完毕
    sleep(SLEEPTIME2);
    m_timeout.restart();
    while (!getSuspend()) {
        if (m_timeout.elapsed() > 3000) {
            qDebug() << "suspend timeout!!!";
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
    m_timeout.restart();

    //恢复运行暂停的按钮
    emit debugStateEnabled(true);
}

void DRobotDebugger::suspendForAPI()
{
    this->SendCmd("suspend\r\n");

    this->m_state = SrcSuspend;
    emit debugStateChanged(this->m_state);
}

bool DRobotDebugger::getSuspend()
{
    QJsonObject rData;
//    d->httpManager->get("/debugger/getSuspend", rData);
    return rData.value("status").toBool();
}

void DRobotDebugger::stepIn(void)
{
    QString url = QString("/debugger/%1").arg("stepIn");
    QJsonObject wData, rData;
//    d->httpManager->post(url, wData, rData);
    this->SendCmd("step\r\n");
    emit stepInChanged(false, false, false);

    //等待step命令下发后，再去查询状态
    sleep(SLEEPTIME1);
    m_timeout.restart();
    while (!getStepIn()) {
        if (m_timeout.elapsed() > 6000) {
            qDebug() << "stepIn timeout!!!";
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
    m_timeout.restart();

    //等待单步完全跳出
    sleep(SLEEPTIME2);

    if (m_state != MobDebugStop) {
        emit stepInChanged(true, false, true);
    }else {
        emit stepInChanged(false, true, true);
    }

    //如果单步执行至完毕
    if (m_isStepFinished) {
        m_isStepFinished = false;
        emit stepInChanged(false, false, false);
        stopMobDebug();
    }
}

void DRobotDebugger::stepOver(void)
{
    QString url = QString("/debugger/%1").arg("stepOver");
    QJsonObject wData, rData;
//    d->httpManager->post(url, wData, rData);
    this->SendCmd("over\r\n");
}

bool DRobotDebugger::getStepIn()
{
    QJsonObject rData;
//    d->httpManager->get("/debugger/stepIn", rData);
    return rData.value("status").toBool();
}

QString DRobotDebugger::getGlobalVal(QString var)
{
    QString value = this->SendCmd("eval " + var + "\r\n");
    value = value.replace("Comm: eval", "");
    value = value.replace("/r/n", "");
    return value;
}

QString DRobotDebugger::changeGlobalVal(QString variable, QString value)
{
    return this->SendCmd("exec " + variable + " = " + value + "\r\n");
}

void DRobotDebugger::setBreakpoint(const QString &fileName, int lineNumber)
{
    QString url = QString("/debugger/%1").arg("setb");
    QJsonObject wData, rData;
    wData.insert("path", fileName);
    wData.insert("line", lineNumber);
//    d->httpManager->post(url, wData, rData);

    this->SendCmd(QString("setb %1 %2\r\n").arg(fileName).arg(lineNumber));
    //下发断点后等待，然后才run
    sleep(SLEEPTIME1);
}

void DRobotDebugger::removeBreakpoint(const QString &fileName, int lineNumber)
{
    QString url = QString("/debugger/%1").arg("delb");
    QJsonObject wData, rData;
    wData.insert("path", fileName);
    wData.insert("line", lineNumber);
//    d->httpManager->post(url, wData, rData);

    this->SendCmd(QString("delb %1 %2\r\n").arg(fileName).arg(lineNumber));
}

void DRobotDebugger::removeAllBreakpoints(void)
{
    QString url = QString("/debugger/%1").arg("delallb");
    QJsonObject wData, rData;
//    d->httpManager->post(url, wData, rData);

    this->SendCmd("delallb\r\n");
}

QString DRobotDebugger::SendCmd(QString cmd)
{
    QString res;

    if (m_process->state() != QProcess::Running) {
        //子进程异常停止
        emit mobDebugInfo("**MainProcess**\nMoDebug Process Crashed!");
        this->stopMobDebug();
        return res;
    }

    res = writeSync(cmd);
    return res;
}

QString DRobotDebugger::writeSync(const QString &sendMsg)
{
    m_process->write(sendMsg.toUtf8());

    //用于暂停后再运行的运动指令耗时较长时跳出,例如Circle3
    m_timeout.restart();

    //等待完成标志
    QString _tempStr;
    while(!_tempStr.contains(">")) {
        const QString str = m_process->readAll();

        if (str.contains("Server closed!")) {
            emit mobDebugInfo("*Server(Close)*\n" + str);
            this->stopMobDebug();
            break;
        }

        //单步运行的特殊处理：解决单步运行时，运行完毕时无法结束的问题
        if (str.contains("Paused at file /dobot/bin/lua/dobot.lua")) {
            emit mobDebugInfo("*Server(Finish)*\nstep script finished!\n");
            m_isStepFinished = true;
            break;
        }

        //单步运行的特殊处理：避免过多打印
        if (str.contains("Paused at")) {
            break;
        }

        if (!str.isEmpty()) {
            //获取到的信息实时抛出到界面
            emit mobDebugInfo("*Server(Info)*\n" + str);
            _tempStr += str;
        }

        //用于运动指令运行过长时的退出
        if (sendMsg.contains("suspend")) {
            if (m_timeout.elapsed() > SLEEPTIME1) {
                qDebug() << "wait suspend timeout!!!";
                break;
            }
        } else if (m_timeout.elapsed() > 2000) {
            qDebug() << "wait return timeout!!!";
            break;
        }

        qDebug() << Q_FUNC_INFO << sendMsg;
        //QApplication::processEvents(QEventLoop::AllEvents, 5);
        sleep(50);
    }

    //返回指令执行过程中的所有打印信息
    return !_tempStr.isEmpty() ? _tempStr.left(_tempStr.indexOf(">")) : "";
}

void DRobotDebugger::onClientMsg(QString msg)
{
    //输出指令错误,截取错误的行号
    //例子：print:ERROR: /dobot/userdata/project/project/DDDDD/src0.lua:5: attempt to call a nil value (global 'sleep')
    if (msg.contains("ERROR:")) {
        //上传错误信息，用于日志记录
        emit errorMsg(msg);

        emit mobDebugInfo("****Client****\n" + msg);
        msg = msg.replace("ERROR:", "");
        msg = msg.mid(msg.indexOf(":") + 1);
        int line = msg.left(msg.indexOf(":")).toInt();
        emit errorLine(line);
        m_hasError = true;
        this->stop();

        return;
    }

    //脚本API下发暂停
    if (msg.contains("command:pause")) {
        suspendForAPI();
        if(!msg.simplified().isEmpty()) {
            emit mobDebugInfo("****Client****\n" + msg);
        }
        msg = msg.replace("command:pause", "");

        return;
    }

    //判断运行完毕
    if (msg.contains("run script finished!")) {
        emit mobDebugInfo("****Client****\n" + msg);
        //this->stopMobDebug();

        return;
    }

    //输出打印信息
    if(!msg.simplified().isEmpty()) {
        emit mobDebugInfo("****Client****\n" + msg);
    }
}

void DRobotDebugger::on1stCursorMsg(QString msg)
{
    //输出第一光标
    if(!msg.simplified().isEmpty()) {
        //识别是哪个线程文件，并兼容旧版本
        if(msg.contains("src")) {
            //只有src0的光标才输出
            if(msg.contains("src0.lua")) {
                msg = msg.mid(msg.indexOf("src0.lua:") + QString("src0.lua:").length());
                int line = msg.toInt();
                emit scriptLineChanged(line);
            }
        }else {
            emit scriptLineChanged(msg.toInt());
        }
    }
}

void DRobotDebugger::on2ndCursorMsg(QString msg)
{
    //输出第二光标
    if(!msg.simplified().isEmpty()) {
        //识别是哪个线程文件，并兼容旧版本
        if(msg.contains("src")) {
            //只有src0的光标才输出
            if(msg.contains("src0.lua")) {
                msg = msg.mid(msg.indexOf("src0.lua:") + QString("src0.lua:").length());
                int line = msg.toInt();
                emit moveLineChanged(line);
            }
        }else {
            emit moveLineChanged(msg.toInt());
        }
    }
}

void DRobotDebugger::onSpecialMsg(QString msg)
{
    qDebug() << Q_FUNC_INFO;
    //输出指令错误,截取错误的行号
    //例子：print:ERROR: /dobot/userdata/project/project/DDDDD/src0.lua:5: attempt to call a nil value (global 'sleep')
    if (msg.contains("ERROR:")) {
        //上传错误信息，用于日志记录
        emit errorMsg(msg);

        emit mobDebugInfo("****Client****\n" + msg);
        msg = msg.replace("ERROR:", "");
        msg = msg.mid(msg.indexOf(":") + 1);
        int line = msg.left(msg.indexOf(":")).toInt();
        emit errorLine(line);
        m_hasError = true;
        this->stop();

        return;
    }

    //脚本API下发暂停
    if (msg.contains("command:pause")) {
        suspendForAPI();
        if(!msg.simplified().isEmpty()) {
            emit mobDebugInfo("****Client****\n" + msg);
        }

        return;
    }

    //判断运行完毕
    if (msg.contains("run script finished!")) {
        emit mobDebugInfo("****Client****\n" + msg);
        this->stopMobDebug();

        return;
    }
}

void DRobotDebugger::stopMobDebug(void)
{
    qDebug() << Q_FUNC_INFO;

    if (this->m_state == MobDebugStop)
        return;

    //断开之前的所有信号连接
    m_process->disconnect();

    //关闭子进程
    m_process->kill();
    m_process->waitForFinished();
    qDebug() << "kill server, MobDebugStop";

    //抛出状态改变信号
    this->m_state = MobDebugStop;
    emit debugStateChanged(this->m_state);
}
