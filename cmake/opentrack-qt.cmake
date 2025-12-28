include_guard(GLOBAL)
set(qt-required-components Core Network Widgets LinguistTools Gui)
set(qt-optional-components SerialPort)
set(qt-imported-targets Qt6::Core Qt6::Gui Qt6::Network Qt6::Widgets)
if(APPLE)
    list(APPEND qt-required-components Multimedia)
    list(APPEND qt-imported-targets Qt6::Multimedia)
endif()
find_package(Qt6 REQUIRED QUIET)
if(TRUE OR NOT APPLE AND NOT WINDOWS)
    set(QT_NO_PRIVATE_MODULE_WARNING ON)
    find_package(Qt6 COMPONENTS GuiPrivate QUIET)
    if(TARGET Qt6::GuiPrivate)
        list(APPEND qt-required-components GuiPrivate)
        list(APPEND qt-imported-targets Qt6::GuiPrivate)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS ${qt-required-components} QUIET)
find_package(Qt6 COMPONENTS ${qt-optional-components} QUIET)

if(WIN32)
    find_package(Qt6Gui REQUIRED COMPONENTS QWindowsIntegrationPlugin)
endif()

function(otr_install_qt_libs)
    foreach(i ${qt-imported-targets})
        #if(NOT TARGET "${i}")
        #    continue()
        #endif()
        otr_install_lib(${i} ".")
    endforeach()
    if(WIN32)
    otr_install_lib(Qt6::QWindowsIntegrationPlugin "./platforms")
    endif()
endfunction()

if(WIN32 OR APPLE)
    otr_install_qt_libs()
endif()

function(otr_qt n)
    if(".${${n}-cc}${${n}-cxx}${${n}-hh}" STREQUAL ".")
        message(FATAL_ERROR "project ${n} not globbed")
   endif()
    qt6_wrap_cpp(${n}-moc ${${n}-hh} OPTIONS --no-notes -I "${CMAKE_CURRENT_BINARY_DIR}" -I "${CMAKE_SOURCE_DIR}")
    qt6_wrap_ui(${n}-uih ${${n}-ui})
    qt6_add_resources(${n}-rcc ${${n}-rc})

    foreach(i moc uih rcc)
        set(${n}-${i} "${${n}-${i}}" PARENT_SCOPE)
        list(APPEND ${n}-all ${${n}-${i}})
    endforeach()
    set(${n}-all "${${n}-all}" PARENT_SCOPE)
endfunction()

function(otr_qt2 n)
    target_include_directories("${n}" SYSTEM PRIVATE
        ${Qt6Core_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS}
    )
    target_compile_definitions("${n}" PRIVATE
        ${Qt6Core_DEFINITIONS} ${Qt6Gui_DEFINITIONS} ${Qt6Widgets_DEFINITIONS} ${Qt6Network_DEFINITIONS}
        -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
        -DQT_MESSAGELOGCONTEXT
    )
    if(CMAKE_COMPILER_IS_GNUCXX)
        set_property(SOURCE ${${n}-moc} ${${n}-rcc}
                     APPEND_STRING PROPERTY COMPILE_FLAGS " -w -Wno-error ")
    endif()
endfunction()

include_directories("${CMAKE_BINARY_DIR}")
