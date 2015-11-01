opentrack_boilerplate(opentrack NO-LIBRARY)

if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console window visible at runtime")
endif()

if(WIN32 AND NOT SDK_CONSOLE_DEBUG)
    set(opentrack-win32-executable WIN32)
else()
    set(opentrack-win32-executable "")
endif()

opentrack_qt(opentrack)
add_executable(opentrack ${opentrack-win32-executable} ${opentrack-all})
opentrack_compat(opentrack)

if(NOT WIN32)
    set_target_properties(opentrack PROPERTIES SUFFIX ".bin")
endif()

target_link_libraries(opentrack
    opentrack-spline-widget
    opentrack-pose-widget
    opentrack-api
    opentrack-compat
    opentrack-version
)

link_with_dinput8(opentrack)

if(APPLE)
    # for process detector
    target_link_libraries(opentrack proc)
endif()

if(LINUX)
    target_link_libraries(opentrack dl)
    # for process detector
    target_link_libraries(opentrack procps)
endif()

set(c-props)
set(l-props)
if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    set(c-props "-fvisibility=hidden -fvisibility-inlines-hidden")
    set(l-props "-Wl,--as-needed")
endif()

set_target_properties(opentrack PROPERTIES
    LINK_FLAGS "${l-props}"
    COMPILE_FLAGS "${c-props}"
)

install(TARGETS opentrack DESTINATION .)
