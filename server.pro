TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -std=c++11

SOURCES += \
#    echo_server_tcp.c \
#    client.c \
    echo_server_select_tcp.cpp

HEADERS += \
    admin.h

DISTFILES += README.md

DISTFILES +=
