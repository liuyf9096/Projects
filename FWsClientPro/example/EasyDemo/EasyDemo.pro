QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DESTDIR = $$PWD/output

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

# add RtWsClient lib as below
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lFWsClient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lFWsClientd
unix:!macx: LIBS += -L$$PWD/lib/ -lFWsClientd

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

