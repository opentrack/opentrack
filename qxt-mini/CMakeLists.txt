if(UNIX OR APPLE)
    set(self opentrack-qxt-mini)
    otr_module(qxt-mini NO-COMPAT BIN)
    otr_pkgconfig(${self} x11 xcb xproto)
    if(APPLE)
        set_property(TARGET ${self}
                     APPEND_STRING PROPERTY
                     LINK_FLAGS " -framework Carbon -framework CoreFoundation ")
    endif()
    target_compile_definitions(${self} PRIVATE -DQXT_BUILD)
endif()
