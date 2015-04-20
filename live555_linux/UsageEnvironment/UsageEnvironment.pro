#-------------------------------------------------
#
# Project created by QtCreator 2014-10-23T19:54:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = UsageEnvironment
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += include \
    ../groupsock/include

SOURCES += \
    HashTable.cpp \
    strDup.cpp \
    UsageEnvironment.cpp

HEADERS += \
    include/Boolean.hh \
    include/HashTable.hh \
    include/strDup.hh \
    include/UsageEnvironment.hh \
    include/UsageEnvironment_version.hh
unix {
    target.path = /usr/lib
    INSTALLS += target
}
