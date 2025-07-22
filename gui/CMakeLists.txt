otr_module(user-interface BIN)

target_link_libraries(${self} PRIVATE
    opentrack-migration
    opentrack-logic
    opentrack-input
    opentrack-spline
    opentrack-pose-widget
)

if(NOT APPLE AND NOT WIN32)
    target_compile_definitions(${self} PRIVATE -DOTR_X11_THREADS)
    otr_pkgconfig(${self} x11)
endif()
