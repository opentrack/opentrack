otr_module(user-interface BIN)

target_link_libraries(opentrack-user-interface
    opentrack-migration
    opentrack-logic
    opentrack-spline
    opentrack-pose-widget
    opentrack-version
)

if(APPLE)
    # for process detector
    target_link_libraries(opentrack-user-interface proc)
endif()

if(LINUX)
    target_link_libraries(opentrack-user-interface dl)
    # for process detector

    # XXX copy-paste from qxt-mini module
    # make a pkg-config wrapper
    # also set ${self} to target's name when invoking `otr_module' -sh 20180527
    otr_pkgconfig(opentrack-user-interface libprocps)
endif()

