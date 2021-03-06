#-------------------------------------------------
#
# Project created by QtCreator 2018-12-11T00:02:37
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../global.pri)
include(../SharedModule/SharedModule.pri)
include(../DatabaseEngine/DatabaseEngine.pri)
include(../PropertiesModule/PropertiesModule.pri)
include(../ActionsModule/ActionsModule.pri)

INCLUDEPATH += ../

TARGET = DatabaseBrowser
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

includeAll()

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
