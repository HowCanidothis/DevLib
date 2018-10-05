#-------------------------------------------------
#
# Project created by QtCreator 2018-06-18T21:26:51
#
#-------------------------------------------------

QT       += core gui

CABOODLE = ../

CONFIG += c++17
QMAKE_CXXFLAGS += /std:c++17

include($$CABOODLE/Global.pri)
include($$CABOODLE/SharedModule/SharedModule.pri)
include($$CABOODLE/CommandsModule/CommandsModule.pri)
include($$CABOODLE/ControllersModule/ControllersModule.pri)
include($$CABOODLE/PropertiesModule/PropertiesModule.pri)
include($$CABOODLE/ResourcesModule/ResourcesModule.pri)
include($$CABOODLE/GraphicsToolsModule/GraphicsToolsModule.pri)
include($$CABOODLE/ProgressModule/ProgressModule.pri)

INCLUDEPATH += $$CABOODLE

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenGLMultithreadingTest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS STATIC_LINK

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        glwidget.cpp \
    glthread.cpp \
    glrender.cpp

HEADERS += \
        glwidget.h \
    glthread.h \
    glrender.h
