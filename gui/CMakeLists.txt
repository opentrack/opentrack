if(MSVC OR (NOT SDK_CONSOLE_DEBUG AND WIN32))
    set(console "")
else()
    set(console WIN32-CONSOLE)
endif()

if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console window visible at runtime")
endif()

opentrack_boilerplate(opentrack EXECUTABLE BIN ${console})

set_target_properties(opentrack PROPERTIES SUFFIX "${opentrack-binary-suffix}")

target_link_libraries(opentrack
    opentrack-logic
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

add_custom_target(fixup-qt-subsystem)

if(MSVC)
    foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets platforms/qwindows)
        add_custom_command(TARGET fixup-qt-subsystem
            POST_BUILD
            COMMAND editbin -nologo -SUBSYSTEM:WINDOWS,5.01 -OSVERSION:5.1 "${i}.dll"
            WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
    endforeach()
endif()

add_dependencies(opentrack fixup-qt-subsystem)
