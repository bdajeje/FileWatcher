#-------------------------------------------------
#
# Project created by QtCreator 2018-07-23T09:05:50
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = FileWatcher
TEMPLATE = app

CONFIG += c++20
QMAKE_CXXFLAGS += -std=c++17 -std=gnu++1z -std=c++1z -fPIC

SOURCES += \
        main.cpp \
        main_window.cpp \
    inotify_thread.cpp

HEADERS += \
        main_window.hpp \
    inotify_thread.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
