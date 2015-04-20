#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T12:36:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = StreamingTest
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += "include/"


TEMPLATE = app


SOURCES += main.cpp


unix:!macx: LIBS += -L$$PWD/../build-isStreaming-Desktop-Debug/ -lisStreaming

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: LIBS += -L$$PWD/../../../../usr/lib/ -lueye_api

INCLUDEPATH += $$PWD/../../../../usr/include
DEPENDPATH += $$PWD/../../../../usr/include
