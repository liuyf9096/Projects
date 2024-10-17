#-------------------------------------------------------
#
# ArduinoPlugin created by LiuYufei 2019-01-29T19:45:00
#
#-------------------------------------------------------

QT       -= gui
QT       += core serialport
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = ArduinoPlugin
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = ArduinoPlugin_d
}

DEFINES += ARDUINO_PLUGIN_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    ArduinoPlugin.cpp \
    DAvrCompiler.cpp \
    DAvrUploader.cpp \
    ArduinoPacket.cpp


HEADERS += \
    ArduinoPlugin.h \
    DAvrCompiler.h \
    DAvrUploader.h \
    ArduinoPacket.h


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
