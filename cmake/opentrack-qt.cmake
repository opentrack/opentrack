include_guard(GLOBAL)
if(WIN32)
    find_package(Qt5Gui REQUIRED COMPONENTS QWindowsIntegrationPlugin)
endif()
set(qt-required-components Core Network Widgets LinguistTools Gui)
set(qt-optional-components SerialPort)
set(qt-imported-targets Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets)
if(APPLE)
    list(APPEND qt-required-components "DBus")
    list(APPEND qt-optional-components "Multimedia")
    list(APPEND qt-imported-targets Qt5::DBus Qt5::Multimedia)
endif()

find_package(Qt5 REQUIRED COMPONENTS ${qt-required-components} QUIET)
find_package(Qt5 COMPONENTS ${qt-optional-components} QUIET)

set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES})
if(APPLE)
    list(APPEND MY_QT_LIBS ${Qt5Multimedia_LIBRARIES} ${Qt5DBus_LIBRARIES})
endif()

function(otr_install_qt_libs)
    foreach(i ${qt-imported-targets})
        if(NOT TARGET "${i}")
            continue()
        endif()
        otr_install_lib(${i} ".")
    endforeach()
    if(WIN32)
    otr_install_lib(Qt5::QWindowsIntegrationPlugin "./platforms")
    endif()
endfunction()

if(WIN32 OR APPLE)
    otr_install_qt_libs()
endif()

function(otr_qt n)
    if(".${${n}-cc}${${n}-cxx}${${n}-hh}" STREQUAL ".")
        message(FATAL_ERROR "project ${n} not globbed")
   endif()
    qt5_wrap_cpp(${n}-moc ${${n}-hh} OPTIONS --no-notes -I "${CMAKE_CURRENT_BINARY_DIR}" -I "${CMAKE_SOURCE_DIR}")
    qt5_wrap_ui(${n}-uih ${${n}-ui})
    qt5_add_resources(${n}-rcc ${${n}-rc})

    foreach(i moc uih rcc)
        set(${n}-${i} "${${n}-${i}}" PARENT_SCOPE)
        list(APPEND ${n}-all ${${n}-${i}})
    endforeach()
    set(${n}-all "${${n}-all}" PARENT_SCOPE)
endfunction()

function(otr_qt2 n)
    target_include_directories("${n}" SYSTEM PRIVATE
        ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
    )
    target_compile_definitions("${n}" PRIVATE
        ${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS}
        -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
        -DQT_MESSAGELOGCONTEXT
    )
    if(CMAKE_COMPILER_IS_GNUCXX)
        set_property(SOURCE ${${n}-moc} ${${n}-rcc}
                     APPEND_STRING PROPERTY COMPILE_FLAGS " -w -Wno-error ")
    endif()
endfunction()

include_directories("${CMAKE_BINARY_DIR}")
