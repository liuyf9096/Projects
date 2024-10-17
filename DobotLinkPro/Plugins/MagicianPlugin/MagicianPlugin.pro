#--------------------------------------------------------
#
# MagicianPlugin created by Liu Yufei 2019-01-29T19:45:00
#
#--------------------------------------------------------

QT       -= gui
QT       += core serialport
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = MagicianPlugin
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = MagicianPlugin_d
}

DEFINES += MAGICIAN_PLUGIN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        MagicianPlugin.cpp \
    MagicianController.cpp \
    MagicianPacket.cpp \
    MWaitForFinish.cpp

HEADERS += \
        MagicianPlugin.h \
    MagicianController.h \
    MagicianPacket.h \
    MWaitForFinish.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

# Connect Dobot-Plugin-Interface Path
win32{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d
}

macx{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d.1.0.0
}

INCLUDEPATH += $$PWD/../../Output \
    $$PWD/../DPluginInterface

DEPENDPATH += $$PWD/../../Output

# Connect DobotDll Path
win32 {
CONFIG(release, debug|release): LIBS += -L$$PWD/DobotDll/ -lDobotDll
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/DobotDll/ -lDobotDll_d
}

macx {
CONFIG(release, debug|release): LIBS += -L$$PWD/DobotDll/ -lDobotDll.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/DobotDll/ -lDobotDll_d.1.0.0
}

INCLUDEPATH += $$PWD/DobotDll
DEPENDPATH += $$PWD/DobotDll
