QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle
TARGET  = rtshowme

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/output_d

SOURCES += \
    main.cpp \
    r_udp_server.cpp

HEADERS += \
    r_udp_server.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

