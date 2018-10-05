#-------------------------------------------------
#
# Project created by QtCreator 2018-01-30T22:51:59
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++17
QMAKE_CXXFLAGS += /std:c++17

CABOODLE = ../

include($$CABOODLE/Global.pri)
include($$CABOODLE/SharedModule/SharedModule.pri)
include($$CABOODLE/SharedGuiModule/SharedGuiModule.pri)
include($$CABOODLE/CommandsModule/CommandsModule.pri)
include($$CABOODLE/ControllersModule/ControllersModule.pri)
include($$CABOODLE/ComputeGraphModule/ComputeGraphModule.pri)
include($$CABOODLE/GraphicsToolsModule/GraphicsToolsModule.pri)
include($$CABOODLE/PropertiesModule/PropertiesModule.pri)
include($$CABOODLE/ResourcesModule/ResourcesModule.pri)

INCLUDEPATH += $$CABOODLE

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sandbox
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS HOME OPENCV STATIC_LINK

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

contains(CONFIG, profile) {
DEFINES += PROFILE_BUILD
}

contains(DEFINES, HOME) {
    win32:CONFIG(release, debug|release): LIBS += -LD:/ThirdParty/opencv_3_3_1/build/x64/vc14/lib/ -lopencv_world331
    else:win32:CONFIG(debug, debug|release): LIBS += -LD:/ThirdParty/opencv_3_3_1/build/x64/vc14/lib/ -lopencv_world331d

    INCLUDEPATH += D:/ThirdParty/opencv_3_3_1/build/include
    DEPENDPATH += D:/ThirdParty/opencv_3_3_1/build/x64/vc14/bin
}
contains(DEFINES, WORK) {
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../ExternalDir/opencv_3_2_vc12_x86_dll_world/x86/vc12/lib/ -lopencv_world320
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../ExternalDir/opencv_3_2_vc12_x86_dll_world/x86/vc12/lib/ -lopencv_world320d

    INCLUDEPATH += $$PWD/../../../ExternalDir/opencv_3_2_vc12_x86_dll_world/include
    DEPENDPATH += $$PWD/../../../ExternalDir/opencv_3_2_vc12_x86_dll_world/include
}
contains(DEFINES, CUDA_TBB_TEST) {
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../BuildingForge/OpencvTBB/opencv3/install/x64/vc14/lib/ -lopencv_world340
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../BuildingForge/OpencvTBB/opencv3/install/x64/vc14/lib/ -lopencv_world340d

    INCLUDEPATH += $$PWD/../../../../BuildingForge/OpencvTBB/opencv3/install/include
    DEPENDPATH += $$PWD/../../../../BuildingForge/OpencvTBB/opencv3/install/include
}

contains(DEFINES, HOME) {
    OpenNIPath = D:\ThirdParty\Opennix64
}
contains(DEFINES, WORK) {
    OpenNIPath = D:\Projects\BuildForge\OpenNI2-master
}

$$(PATH) += ";$$OpenNIPath/Redist"

LIBS += -L$$OpenNIPath/Lib -lOpenNI2
INCLUDEPATH += $$OpenNIPath/Include

includeAll()

