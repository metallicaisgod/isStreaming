#-------------------------------------------------
#
# Project created by QtCreator 2014-10-23T19:50:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = BasicUsageEnvironment
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += include \
    ../UsageEnvironment/include \
    ../groupsock/include

SOURCES += \
    BasicHashTable.cpp \
    BasicTaskScheduler.cpp \
    BasicTaskScheduler0.cpp \
    BasicUsageEnvironment.cpp \
    BasicUsageEnvironment0.cpp \
    DelayQueue.cpp

HEADERS += \
    include/BasicHashTable.hh \
    include/BasicUsageEnvironment.hh \
    include/BasicUsageEnvironment0.hh \
    include/BasicUsageEnvironment_version.hh \
    include/DelayQueue.hh \
    include/HandlerSet.hh
unix {
    target.path = /usr/lib
    INSTALLS += target
}
