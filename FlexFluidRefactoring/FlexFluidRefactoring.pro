#-------------------------------------------------
#
# Project created by QtCreator 2018-01-29T20:55:21
#
#-------------------------------------------------
include (../GraphicsToolsModule/GraphicsToolsModule.pri)
include (../SharedGuiModule/SharedGuiModule.pri)
include (../SharedModule/SharedModule.pri)
include (../PropertiesModule/PropertiesModule.pri)
include (../ResourcesModule/ResourcesModule.pri)
INCLUDEPATH += ../
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FlexFluidRefactoring
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
        main.cpp \
        fluidtestwidget.cpp

HEADERS += \
        fluidtestwidget.h

FORMS +=

win32:CONFIG(release, debug|release): LIBS += -LD:/ThirdParty/opencv_3_3_1/build/x64/vc14/lib/ -lopencv_world331
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ThirdParty/opencv_3_3_1/build/x64/vc14/lib/ -lopencv_world331d

INCLUDEPATH += D:/ThirdParty/opencv_3_3_1/build/include
DEPENDPATH += D:/ThirdParty/opencv_3_3_1/build/x64/vc14/bin
