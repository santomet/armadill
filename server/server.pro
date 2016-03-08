QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle

QMAKE_CXX = g++-4.8
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

TARGET = armadill-server

HEADERS += src/serverconsole.h

SOURCES += \
    src/server.cpp \
    src/serverconsole.cpp


