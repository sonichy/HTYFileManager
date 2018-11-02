QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYFileManager
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    propertydesktop.cpp \
    iconpreview.cpp

HEADERS  += mainwindow.h \
    propertydesktop.h \
    iconpreview.h

FORMS    += mainwindow.ui \
    propertydesktop.ui

RESOURCES += \
    files.qrc
