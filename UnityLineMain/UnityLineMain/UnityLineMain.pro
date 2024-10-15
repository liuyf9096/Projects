#------------------------------------------------------------
#
# Where there is a will, there is a way.
#
# UnityLineMain created by LiuYufei 2021-11-05T16:40:00
#
#------------------------------------------------------------

QT       += core gui widgets sql
QT       += network websockets

CONFIG   += c++11
DEFINES  += QT_DEPRECATED_WARNINGS

TARGET    = unitylinemain
TEMPLATE  = app
RC_FILE   = resource/application.rc

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/../output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../output_d


SOURCES += \
    device/d_track.cpp \
    exception/exception_center.cpp \
    f_common.cpp \
    module/cart/m_cart1.cpp \
    module/cart/m_cart2.cpp \
    module/exit/export_manager.cpp \
    module/exit/m_export1.cpp \
    module/exit/m_export2.cpp \
    module/m_manager_base.cpp \
    module/process/m_aging_test_process.cpp \
    module/process/m_boot_process.cpp \
    module/station/m_station1.cpp \
    module/station/m_station_smear.cpp \
    rt_init.cpp \
    device/rt_device_base.cpp \
    device/rt_device_manager.cpp \
    main.cpp \
    messagecenter/f_jsonrpc_parser.cpp \
    messagecenter/f_message_center.cpp \
    module/cart/carts_manager.cpp \
    module/cart/m_cart.cpp \
    module/exit/m_export.cpp \
    module/entrance/m_import.cpp \
    module/station/m_station.cpp \
    module/module_base.cpp \
    module/module_manager.cpp \
    module/station/station_manager.cpp \
    rack/rt_rack.cpp \
    rack/rt_rack_manager.cpp \
    sample/rt_sample.cpp \
    sample/rt_sample_manager.cpp \
    servers/file/f_file_manager.cpp \
    servers/log/f_log_server.cpp \
    servers/network/f_network_server.cpp \
    servers/settings/f_settings.cpp \
    servers/sql/f_sql_database.cpp \
    servers/sql/f_sql_database_manager.cpp \
    servers/websocket/f_websocket.cpp \
    servers/websocket/f_websocket_manager.cpp \
    servers/websocket/f_websocket_server.cpp \
    views/device_control_page.cpp \
    views/mainwindow.cpp \
    views/debug_record_page.cpp \
    views/device_item/device_item.cpp \
    views/device_item/heart_beat_item.cpp \
    views/device_item/command/debug_command_dialog.cpp \
    views/device_item/command/funciont_value_item.cpp \
    views/sensor_check_page.cpp \
    views/sensors/sensor_item.cpp \
    views/sensors/sensors_form.cpp


HEADERS += \
    device/d_track.h \
    exception/exception_center.h \
    f_common.h \
    module/cart/m_cart1.h \
    module/cart/m_cart2.h \
    module/exit/export_manager.h \
    module/exit/m_export1.h \
    module/exit/m_export2.h \
    module/m_manager_base.h \
    module/process/m_aging_test_process.h \
    module/process/m_boot_process.h \
    module/station/m_station1.h \
    module/station/m_station_smear.h \
    rt_init.h \
    device/rt_device_base.h \
    device/rt_device_manager.h \
    messagecenter/f_jsonrpc_parser.h \
    messagecenter/f_message_center.h \
    module/cart/carts_manager.h \
    module/cart/m_cart.h \
    module/exit/m_export.h \
    module/entrance/m_import.h \
    module/station/m_station.h \
    module/module_base.h \
    module/module_manager.h \
    module/station/station_manager.h \
    rack/rt_rack.h \
    rack/rt_rack_manager.h \
    sample/rt_sample.h \
    sample/rt_sample_manager.h \
    servers/file/f_file_manager.h \
    servers/log/f_log_server.h \
    servers/log/f_log_server_p.h \
    servers/network/f_network_server.h \
    servers/settings/f_settings.h \
    servers/sql/f_sql_database.h \
    servers/sql/f_sql_database_manager.h \
    servers/sql/f_sql_database_p.h \
    servers/websocket/f_websocket.h \
    servers/websocket/f_websocket_manager.h \
    servers/websocket/f_websocket_server.h \
    servers/websocket/f_websocket_server_p.h \
    views/device_control_page.h \
    views/mainwindow.h \
    views/debug_record_page.h \
    views/device_item/device_item.h \
    views/device_item/heart_beat_item.h \
    views/device_item/command/debug_command_dialog.h \
    views/device_item/command/funciont_value_item.h \
    views/sensor_check_page.h \
    views/sensors/sensor_item.h \
    views/sensors/sensors_form.h

FORMS += \
    views/device_control_page.ui \
    views/mainwindow.ui \
    views/debug_record_page.ui \
    views/device_item/device_item.ui \
    views/device_item/heart_beat_item.ui \
    views/device_item/command/debug_command_dialog.ui \
    views/device_item/command/funciont_value_item.ui \
    views/sensor_check_page.ui \
    views/sensors/sensor_item.ui \
    views/sensors/sensors_form.ui

INCLUDEPATH += \
    servers \
    module

RESOURCES += \
    resource/images.qrc

#TRANSLATIONS += \
#    resource/chinese.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
