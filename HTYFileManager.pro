#-------------------------------------------------
#
# Project created by QtCreator 2017-03-28T09:24:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYFileManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    propertydesktop.cpp

HEADERS  += mainwindow.h \
    propertydesktop.h

FORMS    += mainwindow.ui \
    propertydesktop.ui

RESOURCES += \
    files.qrc
