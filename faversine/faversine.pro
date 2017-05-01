QT += core
QT += gui

CONFIG += c++1z

TARGET = faversine
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    data.cpp \
    extraction.cpp \
    interaction.cpp \
    visualisation.cpp \
    vincenty.cpp

DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    range.h \
    data.h \
    extraction.h \
    interaction.h \
    model.h \
    functional.h \
    visualisation.h \
    vincenty.h
