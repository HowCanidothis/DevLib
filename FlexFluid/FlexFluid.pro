#-------------------------------------------------
#
# Project created by QtCreator 2018-01-16T21:17:27
#
#-------------------------------------------------
include(../GraphicsTools/GtCamera.pri)
include(../SharedModule/SharedModule.pri)

INCLUDEPATH += ../

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlexFluid
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        openglwidget.cpp \
    $$PWD/main.cpp \
    mesh.cpp \
    fluid.cpp \
    shadowmap.cpp \
    flex.cpp \
    convex.cpp \
    aabbtree.cpp \
    voxelize.cpp \
    sdf.cpp
HEADERS += \
        openglwidget.h \
    mesh.h \
    fluid.h \
    shadowmap.h \
    flex.h \
    convex.h \
    aabbtree.h \
    voxelize.h \
    trash.h \
    sdf.h

win32: LIBS += -L$$PWD/../../../../BuildingForge/FleX-master/lib/win64/ -lNvFlexDebugCUDA_x64
win32: LIBS += -L$$PWD/../../../../BuildingForge/FleX-master/lib/win64/ -lNvFlexDeviceDebug_x64

INCLUDEPATH += $$PWD/../../../../BuildingForge/FleX-master/include
DEPENDPATH += $$PWD/../../../../BuildingForge/FleX-master/include


