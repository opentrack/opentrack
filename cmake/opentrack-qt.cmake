find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets Gui QUIET)
find_package(Qt5 COMPONENTS SerialPort QUIET)
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS})
set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES})

install(CODE "

    set(run-this FALSE)
    if(\$ENV{USERNAME} STREQUAL \"sthalik\")
        set(run-this TRUE)
    endif()

    if(WIN32 AND run-this)
        if(NOT EXISTS \"${Qt5_DIR}/../../../bin/qmake.exe\")
            message(FATAL_ERROR \"configure qt at least a:${Qt5_DIR} b:\${qt-dir}\")
        endif()

        if(EXISTS \"\${CMAKE_INSTALL_PREFIX}/opentrack.exe\")
            file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}\")
        endif()

        file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\")

        if(NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}\")
            message(FATAL_ERROR \"make sure install dir exists at least\")
        endif()

        set(bin-path \"${Qt5_DIR}/../../../bin\")
        set(platforms-path \"${Qt5_DIR}/../../../plugins/platforms\")

        foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets)
            configure_file(\"\${bin-path}/\${i}.dll\" \"\${CMAKE_INSTALL_PREFIX}/\${i}.dll\" COPYONLY)
        endforeach()

        file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/platforms\")
        configure_file(\"\${platforms-path}/qwindows.dll\" \"\${CMAKE_INSTALL_PREFIX}/platforms/qwindows.dll\" COPYONLY)
    endif()

    if(MSVC)
        foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets platforms/qwindows)
            execute_process(COMMAND editbin -nologo -SUBSYSTEM:WINDOWS,5.01 -OSVERSION:5.1 \"\${i}.dll\" WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\")
        endforeach()
    endif()

")
