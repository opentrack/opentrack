include_guard(GLOBAL)
find_package(Qt5 REQUIRED COMPONENTS Core Network Widgets LinguistTools Gui QUIET)
if(WIN32)
    find_package(Qt5Gui REQUIRED COMPONENTS QWindowsIntegrationPlugin)
endif()
find_package(Qt5 COMPONENTS SerialPort QUIET)

set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES})

function(otr_pdb_for_dll varname path)
    set("${varname}" "" PARENT_SCOPE)
    get_filename_component(dir "${path}" DIRECTORY)
    get_filename_component(name-only "${path}" NAME_WE)
    set(pdb-path "${dir}/${name-only}.pdb")
    if(EXISTS "${pdb-path}")
        set("${varname}" "${pdb-path}" PARENT_SCOPE)
    endif()
endfunction()

function(otr_install_qt_libs)
    if(WIN32)
        foreach(i Qt5::Core Qt5::Gui Qt5::Network Qt5::SerialPort Qt5::Widgets)
            get_property(path TARGET "${i}" PROPERTY LOCATION)
            if("${path}" STREQUAL "")
                message(FATAL_ERROR "${i} ${path}")
            endif()
            install(FILES "${path}" DESTINATION .)
            if(MSVC AND opentrack_install-debug-info)
                otr_pdb_for_dll(pdb-path "${path}")
                if(pdb-path)
                    install(FILES "${pdb-path}" DESTINATION "${opentrack-hier-debug}")
                endif()
            endif()
        endforeach()

        get_property(path TARGET Qt5::QWindowsIntegrationPlugin PROPERTY LOCATION)
        install(FILES "${path}" DESTINATION ./platforms)
        if(MSVC AND opentrack_install-debug-info)
            otr_pdb_for_dll(pdb-path "${path}")
            if(pdb-path)
                install(FILES "${pdb-path}" DESTINATION "${opentrack-hier-debug}")
            endif()
        endif()
    endif()
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
