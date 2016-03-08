QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle

QMAKE_CXX = g++-4.8
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

TARGET = armadill

HEADERS += src/clientconsole.h

SOURCES += \
    src/client.cpp \
    src/clientconsole.cpp

