QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYFileManager
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    propertydesktop.cpp \
    iconpreview.cpp \
    form_disk.cpp

HEADERS  += mainwindow.h \
    propertydesktop.h \
    iconpreview.h \
    form_disk.h

FORMS    += mainwindow.ui \
    propertydesktop.ui \
    form_disk.ui

RESOURCES += \
    files.qrc
