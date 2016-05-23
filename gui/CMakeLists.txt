opentrack_boilerplate(opentrack NO-LIBRARY)

if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console window visible at runtime")
endif()

if(WIN32 AND NOT SDK_CONSOLE_DEBUG)
    set(opentrack-win32-executable WIN32)
else()
    set(opentrack-win32-executable "")
endif()

add_executable(opentrack ${opentrack-win32-executable} ${opentrack-all})

if(NOT WIN32)
    set_target_properties(opentrack PROPERTIES SUFFIX ".bin")
endif()

target_link_libraries(opentrack
    opentrack-spline-widget
    opentrack-pose-widget
    opentrack-version
)

if(APPLE)
    # for process detector
    target_link_libraries(opentrack proc)
endif()

if(LINUX)
    target_link_libraries(opentrack dl)
    # for process detector
    target_link_libraries(opentrack procps)
endif()

opentrack_boilerplate(opentrack STAGE2)

