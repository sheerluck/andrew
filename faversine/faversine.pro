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

QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3 -march=native -mtune=native -pedantic -Wextra -Weffc++ -Wshadow -Wnon-virtual-dtor


CONFIG *= VALGRIND
VALGRIND {
    # Valgrind: compile in Release, with debug symbols and optimized debugging
    CONFIG -= debug
    CONFIG *= release
    QMAKE_CXXFLAGS_RELEASE *= -g
    QMAKE_CXXFLAGS_RELEASE -= -O -O1 -O2 -O3 -Os
    QMAKE_CXXFLAGS_RELEASE *= -O0 -fno-inline
}

