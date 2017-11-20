#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:47:29
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = M3PacketViewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    cpacket.cpp \
    cactivesource.cpp \
    crecorder.cpp \
    cfiledialog.cpp \
    cunitools.cpp \
    cplayer.cpp \
    dialogplayfilter.cpp

HEADERS  += mainwindow.h \
    adjust.h \
    common_header.h \
    Const.h \
    cpacket.h \
    Packet2.h \
    cactivesource.h \
    crecorder.h \
    cfiledialog.h \
    cunitools.h \
    cplayer.h \
    dialogplayfilter.h

FORMS    += mainwindow.ui \
    dialogplayfilter.ui

RESOURCES += \
    resource.qrc

TRANSLATIONS += m3packetviewer_ru.ts
