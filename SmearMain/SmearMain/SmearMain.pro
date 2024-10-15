#------------------------------------------------------------
#
# Where there is a will, there is a way.
#
# SmearMain created by LiuYufei 2021-12-05T19:40:00
#
#------------------------------------------------------------

QT       += core gui widgets sql
QT       += network websockets

CONFIG   += c++11
DEFINES  += QT_DEPRECATED_WARNINGS

TARGET    = smearmain
TEMPLATE  = app
RC_FILE   = resource/application.rc

# Catch Error
win32 {
    QMAKE_LFLAGS_RELEASE += /MAP
    QMAKE_CFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /debug /opt:ref

    LIBS += -lDbgHelp
}

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/../output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../output_d

SOURCES += \
    device/d_print.cpp \
    device/d_sample.cpp \
    device/d_smear.cpp \
    device/d_stain.cpp \
    device/d_track.cpp \
    exception/exception_center.cpp \
    f_common.cpp \
    device/rt_device_base.cpp \
    device/rt_device_manager.cpp \
    main.cpp \
    messagecenter/f_jsonrpc_parser.cpp \
    messagecenter/f_message_center.cpp \
    module/light/m_lights.cpp \
    module/m_manager_base.cpp \
    module/process/m_boot_process.cpp \
    module/process/m_check_sensors_process.cpp \
    module/process/m_clean_all_slots_process.cpp \
    module/process/m_clean_liquid_system_process.cpp \
    module/process/m_clean_smear_blade_process.cpp \
    module/process/m_config_task.cpp \
    module/process/m_drain_filter_process.cpp \
    module/process/m_needle_maintian_process.cpp \
    module/process/m_perfuse_process.cpp \
    module/process/m_process_base.cpp \
    module/process/m_remove_remain_stain_slides.cpp \
    module/process/m_remove_remain_tube.cpp \
    module/process/m_reset_process.cpp \
    module/process/m_shutdown_process.cpp \
    module/process/m_sleep_process.cpp \
    module/process/m_smear_aging_process.cpp \
    module/process/m_stain_aging_process.cpp \
    module/process/process_manager.cpp \
    module/smear/m_sampling.cpp \
    module/smear/m_slidestore.cpp \
    module/smear/smear_manager.cpp \
    module/stain/gripper/gripper_manager.cpp \
    module/stain/gripper/m_gripper1.cpp \
    module/stain/gripper/m_gripper2.cpp \
    module/stain/gripper/m_gripper_base.cpp \
    module/stain/m_stain_cart.cpp \
    module/stain/slot/slot_base.cpp \
    module/stain/slot/slot_c1.cpp \
    module/stain/slot/slot_c2.cpp \
    module/stain/slot/slot_fix.cpp \
    module/stain/slot/slot_fixdry.cpp \
    module/stain/entrance/stain_import.cpp \
    module/stain/entrance/stainonly1.cpp \
    module/stain/entrance/stainonly2.cpp \
    module/stain/entrance/stainonly_base.cpp \
    module/stain/recyclebox/m_recyclebox_mgr.cpp \
    module/stain/recyclebox/recycle_box.cpp \
    module/stain/slot/slot_transfer.cpp \
    module/stain/slot/slot_wash.cpp \
    module/stain/slot/slotgroup_base.cpp \
    module/stain/slot/slotgroup_c1.cpp \
    module/stain/slot/slotgroup_c2.cpp \
    module/stain/slot/slotgroup_fix.cpp \
    module/stain/slot/slotgroup_fixdry.cpp \
    module/stain/slot/slotgroup_transfer.cpp \
    module/stain/slot/slotgroup_wash.cpp \
    module/stain/slot/slots_manager.cpp \
    module/stain/solution/solution_drainer.cpp \
    module/stain/solution/solution_infuser.cpp \
    module/stain/stain_manager.cpp \
    module/track/cart/carts_manager.cpp \
    module/track/cart/m_cart.cpp \
    module/track/cart/m_cart1.cpp \
    module/track/cart/m_cart2.cpp \
    module/track/m_emergency.cpp \
    module/track/m_export.cpp \
    module/track/m_import.cpp \
    module/smear/m_smear.cpp \
    module/module_base.cpp \
    module/module_manager.cpp \
    module/track/track_manager.cpp \
    module/unity/m_unity_task.cpp \
    rack/rt_rack.cpp \
    rack/rt_rack_manager.cpp \
    rt_init.cpp \
    sample/rt_sample.cpp \
    sample/rt_sample_manager.cpp \
    sample/rt_slide.cpp \
    servers/file/f_file_manager.cpp \
    servers/log/f_log_server.cpp \
    servers/network/f_network_server.cpp \
    servers/settings/f_settings.cpp \
    servers/sql/f_sql_database.cpp \
    servers/sql/f_sql_database_manager.cpp \
    servers/websocket/f_websocket.cpp \
    servers/websocket/f_websocket_manager.cpp \
    servers/websocket/f_websocket_server.cpp \
    views/device_setting_page.cpp \
    views/mainwindow.cpp \
    views/debug_record_page.cpp \
    views/device_item/device_item.cpp \
    views/device_item/heart_beat_item.cpp \
    views/device_item/command/debug_command_dialog.cpp \
    views/device_item/command/funciont_value_item.cpp \
    views/pool_form.cpp \
    views/sensor_check_page.cpp \
    views/sensors/sensor_item.cpp \
    views/sensors/sensors_form.cpp \
    views/stain_pool_page.cpp


HEADERS += \
    device/d_print.h \
    device/d_sample.h \
    device/d_smear.h \
    device/d_stain.h \
    device/d_track.h \
    exception/exception_center.h \
    f_common.h \
    device/rt_device_base.h \
    device/rt_device_manager.h \
    messagecenter/f_jsonrpc_parser.h \
    messagecenter/f_message_center.h \
    module/light/m_lights.h \
    module/m_manager_base.h \
    module/process/m_boot_process.h \
    module/process/m_check_sensors_process.h \
    module/process/m_clean_all_slots_process.h \
    module/process/m_clean_liquid_system_process.h \
    module/process/m_clean_smear_blade_process.h \
    module/process/m_config_task.h \
    module/process/m_drain_filter_process.h \
    module/process/m_needle_maintian_process.h \
    module/process/m_perfuse_process.h \
    module/process/m_process_base.h \
    module/process/m_remove_remain_stain_slides.h \
    module/process/m_remove_remain_tube.h \
    module/process/m_reset_process.h \
    module/process/m_shutdown_process.h \
    module/process/m_sleep_process.h \
    module/process/m_smear_aging_process.h \
    module/process/m_stain_aging_process.h \
    module/process/process_manager.h \
    module/smear/m_sampling.h \
    module/smear/m_slidestore.h \
    module/smear/smear_manager.h \
    module/stain/gripper/gripper_manager.h \
    module/stain/gripper/m_gripper1.h \
    module/stain/gripper/m_gripper2.h \
    module/stain/gripper/m_gripper_base.h \
    module/stain/m_stain_cart.h \
    module/stain/slot/slot_base.h \
    module/stain/slot/slot_c1.h \
    module/stain/slot/slot_c2.h \
    module/stain/slot/slot_fix.h \
    module/stain/slot/slot_fixdry.h \
    module/stain/entrance/stain_import.h \
    module/stain/entrance/stainonly1.h \
    module/stain/entrance/stainonly2.h \
    module/stain/entrance/stainonly_base.h \
    module/stain/recyclebox/m_recyclebox_mgr.h \
    module/stain/recyclebox/recycle_box.h \
    module/stain/slot/slot_transfer.h \
    module/stain/slot/slot_wash.h \
    module/stain/slot/slotgroup_base.h \
    module/stain/slot/slotgroup_c1.h \
    module/stain/slot/slotgroup_c2.h \
    module/stain/slot/slotgroup_fix.h \
    module/stain/slot/slotgroup_fixdry.h \
    module/stain/slot/slotgroup_transfer.h \
    module/stain/slot/slotgroup_wash.h \
    module/stain/slot/slots_manager.h \
    module/stain/solution/solution_drainer.h \
    module/stain/solution/solution_infuser.h \
    module/stain/stain_manager.h \
    module/track/cart/carts_manager.h \
    module/track/cart/m_cart.h \
    module/track/cart/m_cart1.h \
    module/track/cart/m_cart2.h \
    module/track/m_emergency.h \
    module/track/m_export.h \
    module/track/m_import.h \
    module/smear/m_smear.h \
    module/module_base.h \
    module/module_manager.h \
    module/track/track_manager.h \
    module/unity/m_unity_task.h \
    rack/rt_rack.h \
    rack/rt_rack_manager.h \
    rt_init.h \
    sample/rt_sample.h \
    sample/rt_sample_manager.h \
    sample/rt_slide.h \
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
    views/device_setting_page.h \
    views/mainwindow.h \
    views/debug_record_page.h \
    views/device_item/device_item.h \
    views/device_item/heart_beat_item.h \
    views/device_item/command/debug_command_dialog.h \
    views/device_item/command/funciont_value_item.h \
    views/pool_form.h \
    views/sensor_check_page.h \
    views/sensors/sensor_item.h \
    views/sensors/sensors_form.h \
    views/stain_pool_page.h


FORMS += \
    views/device_setting_page.ui \
    views/mainwindow.ui \
    views/debug_record_page.ui \
    views/device_item/device_item.ui \
    views/device_item/heart_beat_item.ui \
    views/device_item/command/debug_command_dialog.ui \
    views/device_item/command/funciont_value_item.ui \
    views/pool_form.ui \
    views/sensor_check_page.ui \
    views/sensors/sensor_item.ui \
    views/sensors/sensors_form.ui \
    views/stain_pool_page.ui


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
