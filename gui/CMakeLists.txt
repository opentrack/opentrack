if(MSVC OR (NOT SDK_CONSOLE_DEBUG AND WIN32))
    set(console "")
else()
    set(console WIN32-CONSOLE)
endif()

if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console window visible at runtime")
endif()

otr_module(user-interface EXECUTABLE BIN ${console})

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

