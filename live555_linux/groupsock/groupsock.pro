#-------------------------------------------------
#
# Project created by QtCreator 2014-10-23T19:58:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = groupsock
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += include \
    ../UsageEnvironment/include

SOURCES += \
    GroupEId.cpp \
    Groupsock.cpp \
    GroupsockHelper.cpp \
    IOHandlers.cpp \
    NetAddress.cpp \
    NetInterface.cpp \
    inet.c

HEADERS += \
    groupsock.h \
    include/GroupEId.hh \
    include/Groupsock.hh \
    include/GroupsockHelper.hh \
    include/groupsock_version.hh \
    include/IOHandlers.hh \
    include/NetAddress.hh \
    include/NetCommon.h \
    include/NetInterface.hh \
    include/TunnelEncaps.hh
unix {
    target.path = /usr/lib
    INSTALLS += target
}
