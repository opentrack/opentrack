set(opentrack-perms-file WORLD_READ OWNER_WRITE OWNER_READ GROUP_READ)
set(opentrack-perms-dir WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
set(opentrack-perms-exec "${opentrack-perms-dir}")

set(new-hier-path "#pragma once
#ifndef OPENTRACK_NO_QT_PATH
#   include <QCoreApplication>
#   include <QString>
#   include \"compat/base-path.hpp\"
#   define OPENTRACK_BASE_PATH (application_base_path())
#endif
#define OPENTRACK_LIBRARY_PATH \"${opentrack-hier-path}\"
#define OPENTRACK_DOC_PATH \"${opentrack-hier-doc}\"
#define OPENTRACK_CONTRIB_PATH \"${opentrack-hier-doc}contrib/\"
#define OPENTRACK_I18N_PATH \"${opentrack-i18n-path}\"
")

include_directories("${CMAKE_BINARY_DIR}")

set(hier-path-filename "${CMAKE_BINARY_DIR}/opentrack-library-path.h")
set(orig-hier-path "")
if(EXISTS "${hier-path-filename}")
    file(READ ${hier-path-filename} orig-hier-path)
endif()
if(NOT (orig-hier-path STREQUAL new-hier-path))
    file(WRITE "${hier-path-filename}" "${new-hier-path}")
endif()

function(otr_glob_sources var)
    set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
    file(GLOB ${var}-cc ${dir}/*.cpp ${dir}/*.c)
    file(GLOB ${var}-hh ${dir}/*.h ${dir}/*.hpp)
    file(GLOB ${var}-res ${dir}/*.rc)
    foreach(f ${var}-res)
        set_source_files_properties(${f} PROPERTIES LANGUAGE RC)
    endforeach()
    file(GLOB ${var}-ui ${dir}/*.ui)
    file(GLOB ${var}-rc ${dir}/*.qrc)
    set(${var}-all ${${var}-cc} ${${var}-hh} ${${var}-rc} ${${var}-res})
    foreach(i ui rc res cc hh all)
        set(${var}-${i} "${${var}-${i}}" PARENT_SCOPE)
    endforeach()
endfunction()

function(otr_qt n)
    if(".${${n}-hh}" STREQUAL ".")
        message(FATAL_ERROR "project ${n} not globbed")
    endif()
    qt5_wrap_cpp(${n}-moc ${${n}-hh} OPTIONS --no-notes)
    qt5_wrap_ui(${n}-uih ${${n}-ui})
    qt5_add_resources(${n}-rcc ${${n}-rc})

    foreach(i moc uih rcc)
        set(${n}-${i} "${${n}-${i}}" PARENT_SCOPE)
        list(APPEND ${n}-all ${${n}-${i}})
    endforeach()
    set(${n}-all "${${n}-all}" PARENT_SCOPE)
endfunction()

function(otr_fixup_subsystem n)
    if(MSVC)
        set(subsystem WINDOWS)
        get_property(type TARGET "${n}" PROPERTY TYPE)
        set(loc "$<TARGET_FILE:${n}>")
        if (NOT type STREQUAL "STATIC_LIBRARY")
            add_custom_command(TARGET "${n}"
                               POST_BUILD
                               COMMAND editbin -nologo -SUBSYSTEM:${subsystem},5.01 -OSVERSION:5.1 \"${loc}\")
        endif()
    endif()
endfunction()

function(otr_compat target)
    if(NOT MSVC)
        otr_prop(SOURCE ${${target}-moc} COMPILE_FLAGS "-w -Wno-error")
    endif()
    if(WIN32)
        target_link_libraries(${target} dinput8 dxguid strmiids)
    endif()
    otr_fixup_subsystem(${target})

    set(c-props)
    set(l-props)
    get_property(linker-lang TARGET ${target} PROPERTY LINKER_LANGUAGE)

    if(CMAKE_COMPILER_IS_GNUCXX AND NOT MSVC)
        set(c-props " -fvisibility=hidden")
        if(NOT is-c-only)
            if(NOT WIN32 OR NOT ".${CMAKE_CXX_COMPILER_ID}" STREQUAL ".Clang")
                set(c-props "${c-props} -fuse-cxa-atexit")
            endif()
        endif()
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE AND NOT MSVC)
        set(l-props "-Wl,--as-needed")
    endif()

    otr_prop(TARGET ${target}   COMPIcLE_FLAGS "${c-props} ${arg_COMPILE}"
                                LINK_FLAGS "${l-props} ${arg_LINK}")
endfunction()

include(CMakeParseArguments)

function(otr_install_pdb_current_project target)
    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:${target}>" DESTINATION "${opentrack-hier-debug}" PERMISSIONS ${opentrack-perms-file})
    endif()
endfunction()

function(otr_module n_)
    message(STATUS "module ${n_}")
    cmake_parse_arguments(arg
        "STATIC;NO-COMPAT;BIN;EXECUTABLE;NO-QT;WIN32-CONSOLE;NO-INSTALL;RELINK"
        "LINK;COMPILE"
        "SOURCES"
        ${ARGN}
    )

    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "otr_module bad formals: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    set(n "opentrack-${n_}")

    otr_glob_sources(${n})
    list(APPEND ${n}-all ${arg_SOURCES})

    if(NOT arg_NO-QT)
        otr_qt(${n})
    else()
        set(arg_NO-COMPAT TRUE)
    endif()

    if(NOT WIN32)
        set(subsys "")
    elseif(arg_WIN32-CONSOLE)
        set(subsys "")
    else()
        set(subsys "WIN32")
    endif()

    if(arg_EXECUTABLE)
        add_executable(${n} ${subsys} "${${n}-all}")
    else()
        set(link-mode SHARED)
        if (arg_STATIC)
            set(link-mode STATIC)
        endif()
        add_library(${n} ${link-mode} "${${n}-all}")
    endif()

    set_property(SOURCE ${${n}-moc} ${${n}-uih} ${${n}-rcc} PROPERTY GENERATED TRUE)

    if(NOT arg_RELINK)
        set_property(TARGET ${n} PROPERTY LINK_DEPENDS_NO_SHARED TRUE)
    else()
        set_property(TARGET ${n} PROPERTY LINK_DEPENDS_NO_SHARED FALSE)
    endif()

    if(NOT arg_NO-QT)
        target_link_libraries(${n} ${MY_QT_LIBS})
    endif()

    if(NOT arg_NO-COMPAT)
        target_link_libraries(${n} opentrack-api opentrack-options opentrack-compat)
    endif()

    target_compile_definitions("${n}" PRIVATE "-DOTR_MODULE_NAME=\"${n_}\"")
    string(REPLACE "-" "_" build-n ${n_})
    string(TOUPPER "${build-n}" build-n)
    set_property(TARGET ${n} PROPERTY DEFINE_SYMBOL "BUILD_${build-n}")

    if(arg_STATIC)
        set(arg_NO-INSTALL TRUE)
    endif()

    if(NOT arg_NO-INSTALL)
        if(arg_BIN AND WIN32)
            install(TARGETS "${n}" RUNTIME DESTINATION . PERMISSIONS ${opentrack-perms-exec})
        else()
            install(TARGETS "${n}" ${opentrack-hier-str} PERMISSIONS ${opentrack-perms-exec})
        endif()
        set(opentrack_install-debug-info FALSE CACHE BOOL "Whether to build and install debug info at install time")
        if(opentrack_install-debug-info)
            otr_install_pdb_current_project(${n})
        endif()
    endif()

    otr_compat(${n})
    if(NOT arg_NO-QT)
        otr_i18n_for_target_directory(${n_})
    endif()

    set_property(GLOBAL APPEND PROPERTY opentrack-all-modules "${n}")
    set_property(GLOBAL APPEND PROPERTY opentrack-all-source-dirs "${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(otr_prop type)
    set(files "")
    set(opts ${ARGN})
    # only SOURCE allows for multiple files
    if(".${type}" STREQUAL ".SOURCE")
        while(TRUE)
            # keep popping first element off `opts' and adding to `files`
            list(LENGTH opts len)
            if(NOT "${len}" GREATER 0)
                break()
            endif()
            list(GET opts 0 k)
            string(FIND "${k}" "." idx1)
            string(FIND "${k}" "/" idx2)
            if("${idx1}" GREATER -1 AND "${idx2}" GREATER -1)
                list(REMOVE_AT opts 0)
                list(APPEND files "${k}")
            else()
                # not a pathname
                break()
            endif()
        endwhile()
        # no files, break early
        # happens a few in the codebase
        list(LENGTH files len)
        if(len EQUAL 0)
            return()
        endif()
    else()
        # single file argument
        set(opts "${ARGN}")
        list(GET opts 0 files)
        list(REMOVE_AT opts 0)
    endif()
    # must pass some properties at least
    list(LENGTH opts len)
    if(NOT "${len}" GREATER 0)
        message(FATAL_ERROR "no properties given")
    endif()

    # prop name but no value
    list(LENGTH opts len)
    math(EXPR mod "${len} % 2")
    if(NOT "${mod}" EQUAL 0)
        message(FATAL_ERROR "must specify parameter for each property")
    endif()

    foreach(f ${files})
        set(opts-copy "${opts}")

        while(TRUE)
            list(LENGTH opts-copy len)
            if ("${len}" LESS 2)
                break()
            endif()

            # pop off two elements, set property
            list(GET opts-copy 0 name)
            list(GET opts-copy 1 value)
            list(REMOVE_AT opts-copy 1 0)

            get_property(old "${type}" "${f}" PROPERTY "${name}")
            if(".${old}" STREQUAL ".")
                set(spc "")
            else()
                set(spc " ")
            endif()
            set_property("${type}" "${f}" APPEND_STRING PROPERTY "${name}" "${spc}${value}")
        endwhile()
    endforeach()
endfunction()

function(otr_add_target_dirs var)
    set(globs "")
    foreach(k ${ARGN})
        list(APPEND globs "${k}/CMakeLists.txt")
    endforeach()
    set(projects "")
    file(GLOB projects ${globs})
    list(SORT projects)
    set("${var}" "${projects}" PARENT_SCOPE)
endfunction()
