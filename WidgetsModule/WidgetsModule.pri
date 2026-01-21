includeAll(true)

DEFINES += WIDGETS_MODULE_LIB

win32 {
LIBS *= user32.lib
}

FORMS += \
    $$PWD/TableViews/Header/widgetsresizableheaderattachmentshowcolumnswidget.ui

HEADERS += \
    $$PWD/TableViews/Header/widgetsresizableheaderattachmentshowcolumnswidget.h

SOURCES += \
    $$PWD/TableViews/Header/widgetsresizableheaderattachmentshowcolumnswidget.cpp
