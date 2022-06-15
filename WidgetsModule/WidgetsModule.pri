includeAll(true)

DEFINES += WIDGETS_MODULE_LIB

win32 {
LIBS *= user32.lib
}

HEADERS += \
    $$PWD/Utils/widgetbuilder.h

SOURCES += \
    $$PWD/Utils/widgetbuilder.cpp
