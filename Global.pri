#includeAll([removeMain])
#includes all .cpp .h .qrc .ui files recursivly from catalog where is pro or pri file located (which calls function),
#the function uses DEFINES to watch which pro or pri files already loaded so
#if it called twice, it will not include dublicated files
#removeMain should be always true when it called in pri files and always false in pro, by default it's false

force_debug_info {
DEFINES += QT_PROFILE
}

for(VAR,$$list($$(BUILD_CONFIG))) {
    CONFIG += $$VAR
}

excludeUi {
    DEFINES += FORCE_NO_UI
    export(DEFINES)
}

defineTest(includeAll) {

sources = $$files(*.cpp, true)
removeMain = $$1
isEmpty(removeMain) {
    removeMain = false
}

if($$removeMain) {
    mains = $$files(*main.cpp, true)
    for(val, mains) {
        sources -= $$val
    }
}


headers = $$files(*.h, true)
forms = $$files(*.ui, true)
resources = $$files(*.qrc, true)
precompiled = $$files(*.hpp, true)

defineTest(includeFiles) {
    absfilepath

    excludeUi {
        message(excluding Ui folders)
        for(var, $$1) {
            firstFolder = $$section(var, /, 0, 0)
            !equals(firstFolder, "Ui") {
                absfilepath += $$absolute_path($$var)
                $$2 *= $$absfilepath
            } else {
                message(excluding $$var)
            }
        }
    } else {
        for(var, $$1){
            absfilepath += $$absolute_path($$var)
            $$2 *= $$absfilepath
        }
    }

    export($$2)
    return(true)
}#includeFiles

includeFiles(sources, SOURCES)
includeFiles(headers, HEADERS)
includeFiles(forms, FORMS)
includeFiles(resources, RESOURCES)
includeFiles(precompiled, PRECOMPILED_HEADER)

}#includeAll

win32-g++ {
    QMAKE_CXXFLAGS += -Wno-missing-field-initializers
}

#setBuildDirectory([buildPath])
defineTest(setBuildDirectory) {
    buildPath = $$1

    CONFIG(force_debug_info) {
        message(Profiling)
        BUILDTYPE = profile
        DESTDIR = $$join(buildPath,,,_profile)
    } else {
        CONFIG(debug, debug|release) {
            BUILDTYPE = debug
            DESTDIR = $$join(buildPath,,,_debug)
        } else {
            BUILDTYPE = release
            DESTDIR = $$join(buildPath,,,_release)
        }
    }

    message($$DESTDIR)

    OBJECTS_DIR = $$OUT_PWD/$$BUILDTYPE/obj
    MOC_DIR = $$OUT_PWD/$$BUILDTYPE/moc
    RCC_DIR = $$OUT_PWD/$$BUILDTYPE/rcc
    UI_DIR = $$OUT_PWD/$$BUILDTYPE/ui

    export(DESTDIR)
    export(OBJECTS_DIR)
    export(MOC_DIR)
    export(RCC_DIR)
    export(UI_DIR)
    export(CONFIG)
}

defineTest(makeDeploy) {
    ignoreQt = $$1
    for(FILE,DEPLOY_FILES){
        QMAKE_POST_LINK +=$$quote($(COPY_DIR) $$system_path($${FILE}) $$system_path($${DESTDIR}) $$escape_expand(\n\t))
    }

    for(DIR,DEPLOY_DIRS){
        QMAKE_POST_LINK +=$$quote($(COPY_DIR) $$system_path($${DIR}) $$system_path($${DESTDIR}/$$basename(DIR)) $$escape_expand(\n\t))
    }

    if(!$$ignoreQt) {
        DEPLOY_COMMAND = $$shell_quote($$system_path($$[QT_INSTALL_BINS]/windeployqt))
        DEPLOY_ARG += $$shell_quote($$system_path($$DESTDIR/$$TARGET.exe)) $$EOL
        QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_ARG}
    }

    export(QMAKE_POST_LINK)
}
