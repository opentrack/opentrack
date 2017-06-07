otr_module(user-interface EXECUTABLE BIN)

set_target_properties(opentrack-user-interface PROPERTIES
    SUFFIX "${opentrack-binary-suffix}"
    OUTPUT_NAME "opentrack"
    )

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
    target_link_libraries(opentrack-user-interface procps)
endif()

