#--------------------------------------------------------
#
# MagicDevicePlugin created by LiuYufei 2019-09-16T19:31:00
#
#--------------------------------------------------------

QT       -= gui
QT       += core serialport network
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = MagicDevicePlugin
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = MagicDevicePlugin_d
}

DEFINES += MAGICIAN_PLUGIN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    ../../DobotLink/DError/DError.cpp \
    ../../DobotLink/MessageCenter/DPacket.cpp \
    DProgramDownload.cpp \
    MagicDevicePlugin.cpp

HEADERS += \
    ../../DobotLink/DError/DError.h \
    ../../DobotLink/MessageCenter/DPacket.h \
    DProgramDownload.h \
    MagicDevicePlugin.h

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
win32{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d
}

macx{
CONFIG(release, debug|release): LIBS += -L$$PWD/../../Output/ -lDPluginInterface.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Output_d/ -lDPluginInterface_d.1.0.0
}


# Connect MagicDevice Path
win32 {
CONFIG(release, debug|release): LIBS += -L$$PWD/MagicDevice/ -lMagicDevice
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/MagicDevice/ -lMagicDevice_d
}

macx {
CONFIG(release, debug|release): LIBS += -L$$PWD/MagicDevice/ -lMagicDevice.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/MagicDevice/ -lMagicDevice_d.1.0.0
}

INCLUDEPATH += $$PWD/MagicDevice
DEPENDPATH += $$PWD/MagicDevice
