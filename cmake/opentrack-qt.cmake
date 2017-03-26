find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets Gui QUIET)
find_package(Qt5 COMPONENTS SerialPort Gamepad QUIET)
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
if(is-msvc EQUAL 0)
    set(is-msvc TRUE)
else()
    set(is-msvc FALSE)
endif()

if(MSVC)
    # on .sln generator we have no editbin in path
    if(is-msvc)
        # this is bad but what can we do? searching for vcvarsall.bat
        # would be easier, but then we'd have to differentiate x86/amd64
        # cross tools, etc. which is a can of worms and if/else branches.
        get_filename_component(linker-dir "${CMAKE_LINKER}" DIRECTORY)
        find_file(editbin-executable-filepath "editbin.exe" "${linker-dir}" "${linker-dir}/.." "${linker-dir}/../..")
        otr_escape_string("${editbin-executable-filepath}" editbin-executable)
    else()
        set(editbin-executable "editbin")
    endif()

    install(CODE "
        foreach(i Qt5Core Qt5Gui Qt5Network Qt5SerialPort Qt5Widgets platforms/qwindows)
           execute_process(COMMAND \"${editbin-executable}\" -nologo -SUBSYSTEM:WINDOWS,5.01 -OSVERSION:5.1 \"\${i}.dll\" WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\")
        endforeach()
    ")
endif()

