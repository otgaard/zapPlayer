#-------------------------------------------------
#
# Project created by QtCreator 2016-11-09T08:03:25
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zapPlayer
TEMPLATE = app


SOURCES += main.cpp\
        zapPlayer.cpp \
    QZapWidget.cpp

HEADERS  += zapPlayer.h \
    QZapWidget.h

FORMS    += zapPlayer.ui
