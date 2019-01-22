include(../Shared/Shared.pri)
INCLUDEPATH += ../

QT -= gui

CONFIG += c++11
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    Core/memory/dbmemory.cpp \
    Core/memory/dbmemoryfragment.cpp \
    Core/memory/dbmemorysector.cpp \
    Core/private_objects/dbarraybase.cpp \
    Core/private_objects/dbtableproxy.cpp \
    Core/private_objects/dbtablefielddelegate.cpp \
    dbdatabase.cpp \
    dbtable.cpp \
    dbtablefield.cpp \
    dbtableheader.cpp \
    dbarray.cpp \
    db_decl.cpp

HEADERS += \
    Core/memory/dbmemory.h \
    Core/memory/dbmemoryfragment.h \
    Core/memory/dbmemorysector.h \
    Core/memory/dbmemorydeclarations.h \
    Core/private_objects/dbtableproxy.h \
    Core/private_objects/dbarraybase.h \
    Core/private_objects/dbtablefielddelegate.h \
    Core/dbcontext.h \
    db_decl.h \
    dbdatabase.h \
    dbtable.h \
    dbtablefield.h \
    dbtableheader.h \
    dbarray.h \
    Core/private_objects/dbobjectsarraymanager.h





