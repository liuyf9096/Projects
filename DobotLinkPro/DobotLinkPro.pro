#------------------------------------------------------
#
# DobotLinkPro created by LiuYufei 2019-01-29T17:00:00
#
# DPluginInterface -- plugins' base class
# Plugins          -- user plugins
# DLMaintenance    -- plugins upgrade
#
# plugins:
# MagicDevicePlugin -- Magician(lite), MagicBox, M1
# IndustrialRobotPlugin -- CR5, IR
# ArduinoPlugin     -- AIStarter, MobilePlatform
#
#------------------------------------------------------

TEMPLATE = subdirs

SUBDIRS += \
    DobotLink \
    DLMaintenance \
    Plugins/DPluginInterface \
    Plugins/IndustrialRobotPlugin \
    Plugins/MagicDevicePlugin \
    Plugins/ArduinoPlugin \
#    Plugins/MicrobitPlugin \
    Plugins/DownloadPlugin \
#    Plugins/DemoPlugin



