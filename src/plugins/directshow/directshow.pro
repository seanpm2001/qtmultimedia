TARGET = dsengine
QT += multimedia-private

win32:!qtHaveModule(opengl)|qtConfig(dynamicgl) {
    LIBS_PRIVATE += -lgdi32 -luser32
}

HEADERS += dsserviceplugin.h
SOURCES += dsserviceplugin.cpp

mingw: DEFINES += NO_DSHOW_STRSAFE

include(common/common.pri)
qtConfig(directshow-player): include(player/player.pri)
include(camera/camera.pri)

OTHER_FILES += \
    directshow.json \
    directshow_camera.json

PLUGIN_TYPE = mediaservice
PLUGIN_CLASS_NAME = DSServicePlugin
load(qt_plugin)
