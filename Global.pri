#includeAll([removeMain])
#includes all .cpp .h .qrc .ui files recursivly from catalog where is pro or pri file located (which calls function),
#the function uses DEFINES to watch which pro or pri files already loaded so
#if it called twice, it will not include dublicated files
#removeMain should be always true when it called in pri files and always false in pro, by default it's false

defineTest(includeAll){

sources = $$files(*.cpp, true)
removeMain = $$1
isEmpty(removeMain){
    removeMain = false
}

if($$removeMain){
    mains = $$files(*main.cpp, true)
    for(val, mains){
        sources -= $$val
    }
}


headers = $$files(*.h, true)
forms = $$files(*.ui, true)
resources = $$files(*.qrc, true)
precompiled = $$files(*.hpp, true)

defineTest(includeFiles){
    absfilepath
    for(var, $$1){
        absfilepath += $$absolute_path($$var)
        $$2 *= $$absfilepath
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

