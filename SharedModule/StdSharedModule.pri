DEFINES += NO_QT

PRECOMPILED_HEADER += $$PWD/internal.hpp

HEADERS += $$PWD/process/iprocess.h \
           $$PWD/process/processbase.h \
           $$PWD/process/processfactory.h

SOURCES += $$PWD/process/processbase.cpp \
           $$PWD/process/processfactory.cpp
