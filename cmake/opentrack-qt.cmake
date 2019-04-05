include_guard(GLOBAL)
find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets LinguistTools Gui QUIET)
if(WIN32)
    find_package(Qt5Gui REQUIRED COMPONENTS QWindowsIntegrationPlugin)
endif()
find_package(Qt5 COMPONENTS SerialPort QUIET)

set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES})

function(otr_install_qt_libs)
    foreach(i Qt5::Core Qt5::Gui Qt5::Network Qt5::SerialPort Qt5::Widgets)
        otr_install_lib(${i} ".")
    endforeach()
    otr_install_lib(Qt5::QWindowsIntegrationPlugin "./platforms")
endfunction()

otr_install_qt_libs()

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
    target_include_directories("${n}" PRIVATE SYSTEM
        ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS}
    )
    target_compile_definitions("${n}" PRIVATE
        ${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS}
        -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
        -DQT_MESSAGELOGCONTEXT
    )
endfunction()

include_directories("${CMAKE_BINARY_DIR}")
