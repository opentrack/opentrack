find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets Gui QUIET)
find_package(Qt5 COMPONENTS SerialPort QUIET)
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS})
set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES})

if(WIN32)
    foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets)
        install(FILES "${Qt5_DIR}/../../../bin/${i}.dll" DESTINATION .)
    endforeach()
    install(FILES "${Qt5_DIR}/../../../plugins/platforms/qwindows.dll" DESTINATION "./platforms")
endif()

string(FIND "${CMAKE_GENERATOR}" "Visual Studio " is-msvc)

# on .sln generator we have no editbin path ready

if(MSVC AND NOT is-msvc EQUAL 0)
    install(CODE "
        foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets platforms/qwindows)
           execute_process(COMMAND editbin -nologo -SUBSYSTEM:WINDOWS,5.01 -OSVERSION:5.1 \"\${i}.dll\" WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\")
        endforeach()
    ")
endif()

