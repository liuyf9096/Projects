#--------------------------------------------------------
#
# IndustrialRobotPlugin created by LiuYufei 2020-05-07T19:31:00
#
#--------------------------------------------------------

QT       -= gui
QT       += core serialport network
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = IndustrialRobotPlugin
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = IndustrialRobotPlugin_d
}

DEFINES += MAGICIAN_PLUGIN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    ../../DobotLink/DError/DError.cpp \
    ../../DobotLink/MessageCenter/DPacket.cpp \
    DRobotDebugger.cpp \
    Device.cpp \
    IndustrialRobotPlugin.cpp \
    Mobdebug.cpp \
    Module.cpp \
    RobotStatus.cpp

HEADERS += \
    ../../DobotLink/DError/DError.h \
    ../../DobotLink/MessageCenter/DPacket.h \
    DRobotDebugger.h \
    Device.h \
    IndustrialRobotPlugin.h \
    Mobdebug.h \
    Module.h \
    RobotStatus.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += \
    $$PWD/../../Output \
    $$PWD/../DPluginInterface \
    $$PWD/../../DobotLink

DEPENDPATH += $$PWD/../../Output

include($$PWD/curlnetwork/curlnetwork.pri)

# Connect Dobot-Plugin-Interface Path
win32{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d
}

macx{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d.1.0.0
}

unix{
LIBS += -L$$PWD/../../Output/ -lDPluginInterface
}
