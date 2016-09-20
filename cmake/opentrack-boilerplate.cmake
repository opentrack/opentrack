set(opentrack-perms PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

set(new-hier-path "#pragma once
#ifndef OPENTRACK_NO_QT_PATH

#   include <QCoreApplication>
#   include <QString>

#   define OPENTRACK_BASE_PATH (([]() -> const QString& { \\
        const static QString const__path___ = QCoreApplication::applicationDirPath(); \\
        return const__path___; \\
        })())
#endif
#define OPENTRACK_LIBRARY_PATH \"${opentrack-hier-path}\"
#define OPENTRACK_DOC_PATH \"${opentrack-hier-doc}\"
#define OPENTRACK_CONTRIB_PATH \"${opentrack-hier-doc}contrib/\"
")

set(hier-path-filename "${CMAKE_BINARY_DIR}/opentrack-library-path.h")
set(orig-hier-path "")
if(EXISTS ${hier-path-filename})
    file(READ ${hier-path-filename} orig-hier-path)
endif()
if(NOT (orig-hier-path STREQUAL new-hier-path))
    file(WRITE ${hier-path-filename} "${new-hier-path}")
endif()

function(opentrack_glob_sources var)
    set(dir "${CMAKE_CURRENT_SOURCE_DIR}")
    file(GLOB ${var}-c ${dir}/*.cpp ${dir}/*.c ${dir}/*.h ${dir}/*.hpp)
    file(GLOB ${var}-res ${dir}/*.rc)
    foreach(f ${var}-res)
        set_source_files_properties(${f} PROPERTIES LANGUAGE RC)
    endforeach()
    file(GLOB ${var}-ui ${dir}/*.ui)
    file(GLOB ${var}-rc ${dir}/*.qrc)
    set(${var}-all ${${var}-c} ${${var}-rc} ${${var}-rcc} ${${var}-uih} ${${var}-moc} ${${var}-res})
    foreach(i ui rc res c all)
        set(${var}-${i} "${${var}-${i}}" PARENT_SCOPE)
    endforeach()
endfunction()

function(opentrack_qt n)
    qt5_wrap_cpp(${n}-moc ${${n}-c} OPTIONS --no-notes)
    qt5_wrap_ui(${n}-uih ${${n}-ui})
    qt5_add_resources(${n}-rcc ${${n}-rc})
    foreach(i moc uih rcc)
        set(${n}-${i} "${${n}-${i}}" PARENT_SCOPE)
        list(APPEND ${n}-all ${${n}-${i}})
    endforeach()
    set(${n}-all "${${n}-all}" PARENT_SCOPE)
endfunction()

function(opentrack_fixup_subsystem n)
    if(MSVC)
        if(SDK_CONSOLE_DEBUG)
            set(subsystem CONSOLE)
        else()
            set(subsystem WINDOWS)
        endif()
        set(loc "$<TARGET_FILE:${n}>")
        get_property(type TARGET "${n}" PROPERTY TYPE)
        if (NOT type STREQUAL "STATIC_LIBRARY")
            add_custom_command(TARGET "${n}"
                               POST_BUILD
                               COMMAND editbin -nologo -SUBSYSTEM:${subsystem},5.01 -OSVERSION:5.1 \"${loc}\")
        endif()
    endif()
endfunction()

function(opentrack_compat target)
    if(NOT MSVC)
        set_property(SOURCE ${${target}-moc} APPEND_STRING PROPERTY COMPILE_FLAGS "-w -Wno-error")
    endif()
    if(WIN32)
        target_link_libraries(${target} dinput8 dxguid strmiids)
    endif()
    opentrack_fixup_subsystem(${target})
endfunction()

function(opentrack_sources n ret)
    get_target_property(srcs ${n} SOURCES)
    if(srcs)
        foreach(f ${srcs})
            get_source_file_property(autogen "${f}" GENERATED)
            if(NOT autogen)
                list(APPEND ${ret} "${f}")
            endif()
        endforeach()
    endif()
    set(${ret} "${${ret}}" PARENT_SCOPE)
endfunction()

include(CMakeParseArguments)

function(opentrack_install_sources n)
    opentrack_sources(${n} sources)
    file(RELATIVE_PATH subdir "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (i ${sources})
        install(FILES "${i}" DESTINATION "${opentrack-doc-src-pfx}/${subdir}" ${opentrack-perms})
    endforeach()
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" DESTINATION "${opentrack-doc-src-pfx}/${subdir}" ${opentrack-perms})
endfunction()

function(opentrack_is_target_c_only ret srcs)
    set(val TRUE)
    foreach(i ${srcs})
        get_filename_component(ext "${i}" EXT)
        string(TOLOWER "${ext}" ext)
        if(ext STREQUAL ".cpp")
            set(val FALSE)
            break()
        endif()
    endforeach()
    set(${ret} "${val}" PARENT_SCOPE)
endfunction()

function(opentrack_boilerplate n)
    message(STATUS "module ${n}")
    cmake_parse_arguments(arg
        "STATIC;NO-COMPAT;BIN;EXECUTABLE;NO-QT;WIN32-CONSOLE;NO-INSTALL"
        "LINK;COMPILE"
        "SOURCES"
        ${ARGN}
        )
    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "opentrack_boilerplate bad formals: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    project(${n})
    opentrack_glob_sources(${n})
    opentrack_is_target_c_only(is-c-only "${${n}-all}")
    if(NOT (is-c-only OR arg_NO-QT))
        opentrack_qt(${n})
    else()
        set(arg_NO-QT TRUE)
    endif()
    list(APPEND ${n}-all ${arg_SOURCES})

    if(arg_NO-QT)
        set(arg_NO-COMPAT TRUE)
    endif()

    string(TOUPPER arg_WIN32-CONSOLE "${arg_WIN32-CONSOLE}")

    if(NOT WIN32)
        set(subsys "")
    elseif(MSVC)
        set(subsys "WIN32")
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

    if(NOT arg_NO-QT)
        target_link_libraries(${n} ${MY_QT_LIBS})
    endif()

    if(NOT arg_NO-COMPAT)
        target_link_libraries(${n} opentrack-api opentrack-options opentrack-compat)
    endif()

    if(NOT arg_NO-INSTALL)
        opentrack_install_sources(${n})
    endif()
    opentrack_compat(${n})

    if(CMAKE_COMPILER_IS_GNUCXX)
        set(c-props "-fvisibility=hidden")
        if(NOT is-c-only)
            set(c-props "${c-props} -fuse-cxa-atexit")
        endif()
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
        set(l-props "-Wl,--as-needed")
    endif()

    set_property(TARGET ${n} APPEND_STRING PROPERTY COMPILE_FLAGS "${c-props} ${arg_COMPILE}")
    set_property(TARGET ${n} APPEND_STRING PROPERTY LINK_FLAGS "${l-props} ${arg_LINK}")

    if(NOT arg_STATIC)
        string(REGEX REPLACE "^opentrack-" "" n_ "${n}")
        string(REGEX REPLACE "^(tracker|filter-proto)-" "" n_ "${n_}")
        string(REPLACE "-" "_" n_ ${n_})
        target_compile_definitions(${n} PRIVATE "BUILD_${n_}")

        if(NOT arg_NO-INSTALL)
            if(arg_BIN AND WIN32)
                set(subdir ".")
                install(TARGETS "${n}" RUNTIME DESTINATION . ${opentrack-perms})
            else()
                set(subdir "${opentrack-hier-pfx}")
                install(TARGETS "${n}" ${opentrack-hier-str} ${opentrack-perms})
            endif()
            if(MSVC)
                file(GLOB_RECURSE pdbs "${CMAKE_CURRENT_BINARY_DIR}/*.pdb")
                install(FILES ${pdbs} DESTINATION "${subdir}" ${opentrack-perms})
            endif()
        endif()
    endif()
endfunction()
