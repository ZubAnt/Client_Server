TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
#    echo_server_tcp.c \
#    client.c \
    echo_server_select_tcp.cpp

HEADERS += \
    admin.h

DISTFILES += README.md

DISTFILES +=
