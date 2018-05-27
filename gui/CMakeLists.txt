otr_module(user-interface BIN)

target_link_libraries(${self}
    opentrack-migration
    opentrack-logic
    opentrack-spline
    opentrack-pose-widget
    opentrack-version
)

# for process detector
if(APPLE)
    target_link_libraries(${self} proc)
elseif(LINUX)
    otr_pkgconfig(${self} libprocps)
endif()

