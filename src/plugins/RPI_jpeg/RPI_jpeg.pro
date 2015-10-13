#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T20:58:57
#
#-------------------------------------------------

TARGET = RPI_jpeg


QT       += core

TEMPLATE = lib

LIBS += -lmmal_core -lmmal_util -lmmal_vc_client

DEFINES += RPI_JPEG_LIBRARY

SOURCES += \
    QRPIJpegPlugin.cpp \
    QRPIJpegHandler.cpp \
    brcmjpeg.cpp

target.path = $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target


HEADERS += \
    QRPIJpegPlugin.h \
    QRPIJpegHandler.h \
    brcmjpeg.h
