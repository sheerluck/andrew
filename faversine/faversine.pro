QT += core
QT += gui
QT += concurrent

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


#CONFIG *= VALGRIND
VALGRIND {
    # Valgrind: compile in Release, with debug symbols and optimized debugging
    QMAKE_CXXFLAGS_RELEASE = -g -O0 -fno-inline
}
else {
    QMAKE_CXXFLAGS_RELEASE = -O3 -march=native -mtune=native -mmmx -msse -msse2 -msse3 -pedantic -Wextra -Weffc++ -Wshadow -Wnon-virtual-dtor
}
