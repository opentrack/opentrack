otr_module(compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(${self} PRIVATE rt)
endif()

if(WIN32)
    target_link_libraries(${self} PRIVATE strmiids)
endif()


if(APPLE)
    target_link_libraries(${self} PRIVATE proc)
elseif(LINUX)
    otr_pkgconfig_(has-libproc2 ${self} libproc2)
    if(has-libproc2)
        target_compile_definitions(${self} PUBLIC -DOTR_HAS_LIBPROC2)
    else()
        otr_pkgconfig_(has-libprocps ${self} libprocps)
        if(NOT has-libprocps)
            message(FATAL_ERROR "install libproc2 or libprocps development files")
        endif()
    endif()
endif()
