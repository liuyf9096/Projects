#----------------------------------------------------------
#
# DPluginInterface created by LiuYufei 2019-01-29T18:00:00
#
#----------------------------------------------------------

QT       -= gui
TEMPLATE = lib

CONFIG(release, debug|release): {
DESTDIR = $$PWD/../../Output
TARGET = DPluginInterface
} else:CONFIG(debug, debug|release): {
DESTDIR = $$PWD/../../Output_d
TARGET = DPluginInterface_d
}

DEFINES += DPLUGININTERFACE_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
        DPluginInterface.cpp

HEADERS += \
        DPluginInterface.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#MOC_DIR = $$PWD/../../temp/moc
#RCC_DIR = $$PWD/../../temp/rcc
#UI_DIR = $$PWD/../../temp/ui
#OBJECTS_DIR = $$PWD/../../temp/obj
