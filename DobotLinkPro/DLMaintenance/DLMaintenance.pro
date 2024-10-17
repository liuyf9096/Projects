QT       += core gui widgets
QT       += network

CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/../Output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../Output_d


SOURCES += \
    DNetworkManager.cpp \
    PluginItemForm.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    DNetworkManager.h \
    MainWindow.h \
    PluginItemForm.h

FORMS += \
    MainWindow.ui \
    PluginItemForm.ui


RESOURCES += \
    resource/resource.qrc

RC_FILE = resource/myapp.rc
TRANSLATIONS += resource/dlmaintenance.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

