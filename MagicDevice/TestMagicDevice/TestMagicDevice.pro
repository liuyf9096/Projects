#-------------------------------------------------
#
# Project created by QtCreator 2019-08-29T18:49:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestMagicDevice
CONFIG(release, debug|release): DESTDIR = $$PWD/../output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../output_d

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        MainWindow.cpp

HEADERS += \
        MainWindow.h

FORMS += \
        MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../output/ -lMagicDevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../output_d/ -lMagicDevice_d

INCLUDEPATH += $$PWD/../output_d \
    $$PWD/../include
DEPENDPATH += $$PWD/../output_d
