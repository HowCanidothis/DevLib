build_pass:console {
    warning("QAxServer applications cannot be console applications.")
    warning("Remove 'console' from your CONFIG.")
}

# Do not link qtmain.lib
QMAKE_LIBS_QT_ENTRY =

isEmpty(ACTIVEQT_VERSION):ACTIVEQT_VERSION = 1.0

qtPrepareTool(QMAKE_IDC, idc)
contains(TEMPLATE, "vc.*") {
    ACTIVEQT_IDC = $${QMAKE_IDC}
    ACTIVEQT_IDL = $${QMAKE_IDL}
    ACTIVEQT_TARGET = \"$(TargetPath)\"
    ACTIVEQT_NEWLINE = $$escape_expand(\\n\\t)
    ACTIVEQT_OUTPUT = $(IntDir)$${TARGET}
    ACTIVEQT_TLBOUT = \"$(TargetDir)/$${TARGET}.tlb\"
    GENERATED += $${OBJECTS_DIR}/$${TARGET}.idl $${ACTIVEQT_TLBOUT}
} else {
    ACTIVEQT_IDC = -$(IDC)
    ACTIVEQT_IDL = -$(IDL)
    ACTIVEQT_NEWLINE = $$escape_expand(\\n\\t)
    ACTIVEQT_TARGET = $(DESTDIR_TARGET)
    ACTIVEQT_OUTPUT = $(OBJECTS_DIR)/$${TARGET}
    isEmpty(DESTDIR) {
        ACTIVEQT_TLBOUT = $${TARGET}.tlb
    } else {
        ACTIVEQT_TLBOUT = $${DESTDIR}/$${TARGET}.tlb
    }
}

!isEmpty(QMAKE_POST_LINK):QMAKE_POST_LINK += $$quote($$ACTIVEQT_NEWLINE)
QMAKE_POST_LINK += $$quote($$ACTIVEQT_IDC $${ACTIVEQT_TARGET} /idl $${ACTIVEQT_OUTPUT}.idl -version $${ACTIVEQT_VERSION}$${ACTIVEQT_NEWLINE})
!isEmpty(RC_FILE) {
    QMAKE_POST_LINK += $$quote($$ACTIVEQT_IDL "$${ACTIVEQT_OUTPUT}.idl" /nologo /tlb "$${ACTIVEQT_OUTPUT}.tlb"$$ACTIVEQT_NEWLINE)
    QMAKE_POST_LINK += $$quote($$ACTIVEQT_IDC $${ACTIVEQT_TARGET} /tlb $${ACTIVEQT_OUTPUT}.tlb$$ACTIVEQT_NEWLINE)
} else {
    QMAKE_POST_LINK += $$quote($$ACTIVEQT_IDL "$${ACTIVEQT_OUTPUT}.idl" /nologo /tlb "$${ACTIVEQT_TLBOUT}"$$ACTIVEQT_NEWLINE)
    message("No rc-file linked into project; type library will be a separate file.")
}
!qaxserver_no_register: \
    QMAKE_POST_LINK += $$quote($$ACTIVEQT_IDC $${ACTIVEQT_TARGET} /regserver)
QMAKE_CLEAN += $${ACTIVEQT_OUTPUT}.idl $${ACTIVEQT_OUTPUT}.tlb

