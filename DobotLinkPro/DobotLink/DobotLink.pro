#---------------------------------------------------
#
# DobotLink created by LiuYufei 2019-01-29T17:30:00
# Where there is a will, there is a way.
#
#---------------------------------------------------

QT       += core gui widgets
QT       += network websockets serialport
QT       += multimedia multimediawidgets

TARGET = DobotLink
TEMPLATE = app

# DESTDIR
CONFIG(release, debug|release): DESTDIR = $$PWD/../Output
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../Output_d

CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += \
    Module/DNetworkManager.cpp \
    Upgrade/DUpgrade.cpp \
    Views/DDownloadTestForm.cpp \
    Views/DUpgradeDialog.cpp \
    Views/DfuDownloadTestForm.cpp \
    main.cpp \
    DobotLinkMain.cpp \
    DError/DError.cpp \
    DelayToQuit/DelayToQuit.cpp \
    MessageCenter/DMessageCenter.cpp \
    MessageCenter/DPacket.cpp \
    PluginManager/DPluginManager.cpp \
    Module/DLogger.cpp \
    Module/DSerialPort.cpp \
    Module/DOpenFile.cpp \
    Module/DSettings.cpp \
    Module/DTcpSocketServer.cpp \
    Module/DWebSocketServer.cpp \
    Views/DAboutUsDialog.cpp \
    Views/DArduinoTestForm.cpp \
    Views/DMicrobitTestForm.cpp \
    Views/DSerialTestForm.cpp \
    Views/DMagicianTestForm.cpp \
    Views/DDownloadDialog.cpp \
    Views/DSendUserPacketDialog.cpp \
    Views/DLocalMonitorForm.cpp \
    Views/DRemoteMonitorForm.cpp

HEADERS += \
    DobotLinkMain.h \
    DError/DError.h \
    DelayToQuit/DelayToQuit.h \
    MessageCenter/DMessageCenter.h \
    MessageCenter/DPacket.h \
    Module/DNetworkManager.h \
    PluginManager/DPluginManager.h \
    Module/DLogger.h \
    Module/DSerialPort.h \
    Module/DOpenFile.h \
    Module/DSettings.h \
    Module/DTcpSocketServer.h \
    Module/DWebSocketServer.h \
    Upgrade/DUpgrade.h \
    Views/DAboutUsDialog.h \
    Views/DArduinoTestForm.h \
    Views/DDownloadTestForm.h \
    Views/DMicrobitTestForm.h \
    Views/DSerialTestForm.h \
    Views/DMagicianTestForm.h \
    Views/DDownloadDialog.h \
    Views/DSendUserPacketDialog.h \
    Views/DLocalMonitorForm.h \
    Views/DRemoteMonitorForm.h \
    Views/DUpgradeDialog.h \
    Views/DfuDownloadTestForm.h

FORMS += \
    DobotLinkMain.ui \
    Views/DAboutUsDialog.ui \
    Views/DArduinoTestForm.ui \
    Views/DDownloadDialog.ui \
    Views/DDownloadTestForm.ui \
    Views/DLocalMonitorForm.ui \
    Views/DMicrobitTestForm.ui \
    Views/DRemoteMonitorForm.ui \
    Views/DSendUserPacketDialog.ui \
    Views/DSerialTestForm.ui \
    Views/DMagicianTestForm.ui \
    Views/DUpgradeDialog.ui \
    Views/DfuDownloadTestForm.ui

RESOURCES += \
    resource/translation.qrc \
    resource/image.qrc


TRANSLATIONS += resource/dobotlink.ts
RC_FILE = resource/myapp.rc
ICON = dobotlink.icns


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Connect Dobot-Plugin-Interface Path
win32 {
CONFIG(release, debug|release): LIBS += -L$$PWD/../Output/ -lDPluginInterface
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Output_d/ -lDPluginInterface_d
}

macx {
CONFIG(release, debug|release): LIBS += -L$$PWD/../Output/ -lDPluginInterface.1.0.0
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Output_d/ -lDPluginInterface_d.1.0.0
}

INCLUDEPATH += $$PWD/../Output \
    $$PWD/../Plugins/DPluginInterface

DEPENDPATH += $$PWD/../Output

#MOC_DIR = $$PWD/../temp/moc
#RCC_DIR = $$PWD/../temp/rcc
#UI_DIR = $$PWD/../temp/ui
#OBJECTS_DIR = $$PWD/../temp/obj
