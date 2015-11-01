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
target_link_libraries(opentrack opentrack-api opentrack-version opentrack-pose-widget opentrack-spline-widget)
if(UNIX OR APPLE)
    target_link_libraries(opentrack opentrack-qxt-mini)
endif()
link_with_dinput8(opentrack)
target_link_libraries(opentrack ${MY_QT_LIBS})

if(APPLE)
    # for process detector
    target_link_libraries(opentrack proc)
endif()

if(LINUX)
    # for process detector
    target_link_libraries(opentrack procps)
endif()

install(TARGETS opentrack DESTINATION .)
