QT += core gui widgets network

CONFIG += c++11

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/output_d

SOURCES += \
    change_decription_dialog.cpp \
    f_common.cpp \
    f_settings.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    change_decription_dialog.h \
    f_common.h \
    f_settings.h \
    mainwindow.h

FORMS += \
    change_decription_dialog.ui \
    mainwindow.ui

RC_FILE = app.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
