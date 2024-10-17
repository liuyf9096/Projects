#-----------------------------------------------------
#
# Project created by Liu yufei 2019-08-09T17:11:20
# available device: Magician,MagicianLite,MagicBox,M1
#
#-----------------------------------------------------

QT       -= gui
QT       += serialport network

CONFIG(release, debug|release): {
    DESTDIR = $$PWD/output
    TARGET = MagicDevice
} else:CONFIG(debug, debug|release): {
    DESTDIR = $$PWD/output_d
    TARGET = MagicDevice_d
}

TEMPLATE = lib
CONFIG += c++11
DEFINES += MAGICDEVICE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ActionTimerManager.cpp \
    MagicDevice.cpp \
    MessageHandler.cpp \
    packet/DM1Protocol.cpp \
    packet/DMagicianProtocol.cpp \
    packet/DPacket.cpp

HEADERS += \
    MagicDevice_p.h \
    include/MagicDevice.h \
    DError.h \
    ActionTimerManager.h \
    MessageHandler.h \
    packet/DM1Protocol.h \
    packet/DMagicianProtocol.h \
    packet/DPacket.h

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/packet

unix {
    target.path = /usr/lib
    INSTALLS += target
}
