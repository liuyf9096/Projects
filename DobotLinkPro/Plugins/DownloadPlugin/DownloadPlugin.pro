#------------------------------------------------------
#
# DownloadPlugin created by LiuYufei 2019-05-9T10:08:00
#
#------------------------------------------------------

QT       -= gui
QT       += core serialport
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = DownloadPlugin
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = DownloadPlugin_d
}

DEFINES += DEMO_PLUGIN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    ../../DobotLink/DError/DError.cpp \
    ../../DobotLink/MessageCenter/DPacket.cpp \
    DDfufile.cpp \
    DMcuisp.cpp \
    DownloadPlugin.cpp

HEADERS += \
    ../../DobotLink/DError/DError.h \
    ../../DobotLink/MessageCenter/DPacket.h \
    DDfufile.h \
    DMcuisp.h \
    DownloadPlugin.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += \
    $$PWD/../../Output \
    $$PWD/../DPluginInterface \
    $$PWD/../../DobotLink

DEPENDPATH += $$PWD/../../Output


# Connect Dobot-Plugin-Interface Path
win32 {
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d
}

macx {
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d.1.0.0
}





