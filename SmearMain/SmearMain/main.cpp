#include "f_common.h"
#include "rt_init.h"
#include "views/mainwindow.h"
#include "module/module_manager.h"

#include <QApplication>
#include <QTranslator>

//#define CATCH_DUMP

#ifdef CATCH_DUMP
#if defined(Q_OS_WIN)
#include <Windows.h>
#include <dbghelp.h>
#include <string>

#include <QDebug>
#include <QDir>
#include <QDateTime>
#endif

#if defined(Q_OS_WIN)
/* Catch Error */
long ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    QString currPath = QDir::currentPath() + "/dmp/";
    {
        /* Create Dump directory */
        QDir *dmp = new QDir;
        bool exist = dmp->exists(currPath);
        if(exist == false) {
            dmp->mkdir(currPath);
        }
    }
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy_MM_dd_hh_mm_ss");
    QString time = current_date + ".dmp";
    EXCEPTION_RECORD *record = pException->ExceptionRecord;
    QString errCode(QString::number(record->ExceptionCode, 16));
    QString errAddr(QString::number((uint)record->ExceptionAddress, 16));
    QString errFlag(QString::number(record->ExceptionFlags, 16));
    QString errPara(QString::number(record->NumberParameters, 16));
    qDebug() << "errCode: " << errCode;
    qDebug() << "errAddr: " << errAddr;
    qDebug() << "errFlag: " << errFlag;
    qDebug() << "errPara: " << errPara;
    HANDLE hDumpFile = CreateFile((LPCWSTR)QString(currPath + time).utf16(),
                                  GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDumpFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ClientPointers = TRUE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
        CloseHandle(hDumpFile);
    } else {
        qDebug()<<"hDumpFile == null";
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
#endif

int main(int argc, char *argv[])
{
#ifdef CATCH_DUMP
#ifdef Q_OS_WIN
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
#endif
#endif

    QApplication app(argc, argv);
    FCommon::printSystemInfo();

    /* servers */
    if (argc > 1) {
        if (qstrcmp(argv[1], "-verbose") == 0 || qstrcmp(argv[1], "-v") == 0) {
            FCommon::GetInstance()->setVerbose();
        }
    }

#ifdef Q_OS_WIN
    FCommon::GetInstance()->setVerbose();
#endif

    servers_init();

#if 0
    /* languages */
    QTranslator translator;
    if (translator.load("://chinese.qm")) {
        app.installTranslator(&translator);
    }
#endif

#ifdef Q_OS_WIN
    MainWindow w;
    w.show();
#endif

    return app.exec();
}


