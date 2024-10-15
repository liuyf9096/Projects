QT      -= gui
QT      += websockets network
TEMPLATE = lib
CONFIG  += c++11 skip_target_version_ext
DEFINES += FWSSERVER_LIBRARY

VERSION = 0.1.1.6

QMAKE_TARGET_PRODUCT = Websocket Communication Server
QMAKE_TARGET_COMPANY = ReeToo company Ltd
#QMAKE_TARGET_DESCRIPTION = record all the debug informations
QMAKE_TARGET_COPYRIGHT = Copyright(C) 2015-2022 the ReeToo company Ltd

CONFIG(release, debug|release): {
    DESTDIR = $$PWD/lib
    TARGET = FWsServer
} else:CONFIG(debug, debug|release): {
    DESTDIR = $$PWD/lib
    TARGET = FWsServerd
}

SOURCES += \
    src/f_jsonrpc_parser.cpp \
    src/f_network_server.cpp \
    src/f_ws_server.cpp


HEADERS += \
    include/f_jsonrpc_parser.h \
    include/f_ws_server.h \
    src/f_network_server.h \
    src/f_ws_server_p.h


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
