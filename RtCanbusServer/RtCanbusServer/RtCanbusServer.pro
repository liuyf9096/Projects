#------------------------------------------------------------
#
# Where there is a will, there is a way.
#
# RtCanbusServer created by LiuYufei 2021-10-25T16:40:00
#
#------------------------------------------------------------

QT       += core gui
QT       += widgets
QT       += sql
QT       += network websockets      # used for tcp/ip websocket http communication
QT       += serialbus               # used for canbus

win32 {
}

CONFIG += c++11
DEFINES  += \
    QT_DEPRECATED_WARNINGS \
    CAN_VERBOSE \
    CENTER_VERBOSE

TARGET    = rtcanbusserver

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/../output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../output_d


SOURCES += \
    device/device_protocol.cpp \
    device/rt_device.cpp \
    main.cpp \
    f_common.cpp \
    rt_init.cpp \
    servers/network/f_network_server.cpp \
    tasks/upgrader.cpp \
    views/mainwindow.cpp \
    device/rt_device_base.cpp \
    device/rt_device_manager.cpp \
    messagecenter/f_jsonrpc_parser.cpp \
    messagecenter/f_message_center.cpp \
    servers/file/f_file_manager.cpp \
    servers/sql/f_sql_database.cpp \
    servers/sql/f_sql_database_manager.cpp \
    tasks/task_handler.cpp \
    servers/canbus/f_canbus_device.cpp \
    servers/canbus/f_canbus_protocol.cpp \
    servers/canbus/f_canbus_server.cpp \
    servers/log/f_log_server.cpp \
    servers/settings/f_settings.cpp \
    servers/websocket/f_websocket_server.cpp

HEADERS += \
    device/device_protocol.h \
    device/rt_device.h \
    f_common.h \
    rt_init.h \
    servers/network/f_network_server.h \
    tasks/upgrader.h \
    views/mainwindow.h \
    device/rt_device_base.h \
    device/rt_device_manager.h \
    messagecenter/f_jsonrpc_parser.h \
    messagecenter/f_message_center.h \
    servers/file/f_file_manager.h \
    servers/sql/f_sql_database.h \
    servers/sql/f_sql_database_manager.h \
    servers/sql/f_sql_database_p.h \
    tasks/task_handler.h \
    servers/canbus/f_canbus_device.h \
    servers/canbus/f_canbus_protocol.h \
    servers/canbus/f_canbus_server.h \
    servers/log/f_log_server.h \
    servers/log/f_log_server_p.h \
    servers/settings/f_settings.h \
    servers/websocket/f_websocket_server.h \
    servers/websocket/f_websocket_server_p.h

FORMS += \
    views/mainwindow.ui

INCLUDEPATH += \
    servers


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
