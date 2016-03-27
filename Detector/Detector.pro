#-------------------------------------------------
#
# Project created by QtCreator 2016-03-27T14:00:07
#
#-------------------------------------------------

QT       -= gui

TARGET = Detector
TEMPLATE = lib
CONFIG += staticlib

SOURCES += detector.cpp

HEADERS += detector.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
