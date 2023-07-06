otr_module(user-interface BIN)

target_link_libraries(${self}
    opentrack-migration
    opentrack-logic
    opentrack-spline
    opentrack-pose-widget
)

# for process detector
if(APPLE)
    target_link_libraries(${self} proc)
elseif(LINUX)
    otr_pkgconfig_(has-libproc2 ${self} libproc2)
    if(has-libproc2)
        target_compile_definitions(${self} PRIVATE -DOTR_HAS_LIBPROC2)
    else()
        otr_pkgconfig(${self} libprocps)
    endif()
endif()

if(NOT APPLE AND NOT WIN32)
    target_compile_definitions(${self} PRIVATE -DOTR_X11_THREADS)
    otr_pkgconfig(${self} x11)
endif()
