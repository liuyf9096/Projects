SOURCES += \
    $$PWD/CurlNetworkManager.cpp \
    $$PWD/CurlNetworkRequest.cpp \
    $$PWD/CurlNetworkReply.cpp

HEADERS += \
    $$PWD/CurlNetworkManager.h \
    $$PWD/CurlNetworkRequest.h \
    $$PWD/CurlNetworkReply.h

win32: LIBS += -L$$PWD/lib/ -lcurldll
macx: LIBS += -L$$PWD/lib_mac/ -lcurl.4

INCLUDEPATH += $$PWD/curl
DEPENDPATH += $$PWD/curl
