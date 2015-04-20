#-------------------------------------------------
#
# Project created by QtCreator 2014-10-24T14:14:37
#
#-------------------------------------------------

QT       -= core gui

TARGET = isStreaming
TEMPLATE = lib

DEFINES += ISSTREAMING_EXPORTS

LIBS += -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale \

INCLUDEPATH += UsageEnvironment/include \
    BasicUsageEnvironment/include \
    groupsock/include \
    liveMedia/include \

SOURCES += \
    BasicTaskScheduler.cpp \
    isStreaming.cpp \
    JPEGVideoRTPSink.cpp \
    MJPEGSource.cpp

HEADERS += \
    BasicTaskScheduler.h \
    isStreaming.h \
    JPEGVideoRTPSink.h \
    MJPEGSource.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


unix:!macx: LIBS += -L$$PWD/../build-live555-Desktop-Release/ -llive555

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

unix:!macx: PRE_TARGETDEPS += $$PWD/../build-live555-Desktop-Release/liblive555.a

unix:!macx: LIBS += -L$$PWD/../live555_linux/build-groupsock-Desktop-Release/ -lgroupsock

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

unix:!macx: PRE_TARGETDEPS += $$PWD/../live555_linux/build-groupsock-Desktop-Release/libgroupsock.a

unix:!macx: LIBS += -L$$PWD/../live555_linux/build-UsageEnvironment-Desktop-Release/ -lUsageEnvironment

INCLUDEPATH += $$PWD/../live555_linux/build-UsageEnvironment-Desktop-Release
DEPENDPATH += $$PWD/../live555_linux/build-UsageEnvironment-Desktop-Release

unix:!macx: PRE_TARGETDEPS += $$PWD/../live555_linux/build-UsageEnvironment-Desktop-Release/libUsageEnvironment.a

unix:!macx: LIBS += -L$$PWD/../live555_linux/build-BasicUsageEnvironment-Desktop-Release/ -lBasicUsageEnvironment

INCLUDEPATH += $$PWD/../live555_linux/build-BasicUsageEnvironment-Desktop-Release
DEPENDPATH += $$PWD/../live555_linux/build-BasicUsageEnvironment-Desktop-Release

unix:!macx: PRE_TARGETDEPS += $$PWD/../live555_linux/build-BasicUsageEnvironment-Desktop-Release/libBasicUsageEnvironment.a
