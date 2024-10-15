QT      -= gui
QT      += websockets network
TEMPLATE = lib
CONFIG  += c++11 skip_target_version_ext
DEFINES += FWSCLINET_LIBRARY

VERSION = 0.2.1.11

QMAKE_TARGET_PRODUCT = Websocket Communication Client
QMAKE_TARGET_COMPANY = ReeToo company Ltd
#QMAKE_TARGET_DESCRIPTION = record all the debug informations
QMAKE_TARGET_COPYRIGHT = Copyright(C) 2015-2023 the ReeToo company Ltd

CONFIG(release, debug|release): {
    DESTDIR = $$PWD/lib
    TARGET = FWsClient
} else:CONFIG(debug, debug|release): {
    DESTDIR = $$PWD/lib
    TARGET = FWsClientd
}

SOURCES += \
    src/f_jsonrpc_parser.cpp \
    src/f_ws_client.cpp


HEADERS += \
    include/f_jsonrpc_parser.h \
    include/f_ws_client.h \
    src/f_ws_client_p.h


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
