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
#define OPENTRACK_I18N_PATH \"${opentrack-i18n-path}\"
")

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

function(otr_qt n)
    qt5_wrap_cpp(${n}-moc ${${n}-c} OPTIONS --no-notes)
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

function(otr_compat target)
    if(NOT MSVC)
        set_property(SOURCE ${${target}-moc} APPEND_STRING PROPERTY COMPILE_FLAGS "-w -Wno-error")
    endif()
    if(WIN32)
        target_link_libraries(${target} dinput8 dxguid strmiids)
    endif()
    otr_fixup_subsystem(${target})
endfunction()

include(CMakeParseArguments)

function(otr_is_target_c_only ret srcs)
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

function(otr_install_pdb_current_project)
    if(MSVC AND FALSE)
        file(GLOB_RECURSE pdbs "${CMAKE_CURRENT_BINARY_DIR}/*.pdb")
        install(FILES ${pdbs} DESTINATION "${subdir}" ${opentrack-perms})
    endif()
endfunction()

function(otr_module n)
    message(STATUS "module ${n}")
    cmake_parse_arguments(arg
        "STATIC;NO-COMPAT;BIN;EXECUTABLE;NO-QT;WIN32-CONSOLE;NO-INSTALL"
        "LINK;COMPILE"
        "SOURCES"
        ${ARGN}
        )
    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "otr_module bad formals: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    set(n_orig "${n}")
    set(n "opentrack-${n}")

    project(${n})
    otr_glob_sources(${n})
    otr_is_target_c_only(is-c-only "${${n}-all}")
    if(NOT (is-c-only OR arg_NO-QT))
        otr_qt(${n})
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
    set_property(TARGET ${n} PROPERTY PREFIX "")

    if(NOT arg_NO-QT)
        target_link_libraries(${n} ${MY_QT_LIBS})
    endif()

    if(NOT arg_NO-COMPAT)
        target_link_libraries(${n} opentrack-api opentrack-options opentrack-compat)
    endif()

    otr_compat(${n})

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
        string(REPLACE "-" "_" n_ ${n_})
        string(TOUPPER "${n_}" n__)
        target_compile_definitions(${n} PRIVATE "BUILD_${n__}")

        if(NOT arg_NO-INSTALL)
            if(arg_BIN AND WIN32)
                set(subdir ".")
                install(TARGETS "${n}" RUNTIME DESTINATION . ${opentrack-perms})
            else()
                set(subdir "${opentrack-hier-pfx}")
                install(TARGETS "${n}" ${opentrack-hier-str} ${opentrack-perms})
            endif()
            otr_install_pdb_current_project()
        endif()
    else()
        target_compile_definitions(${n} PRIVATE "STATIC_LIBRARY=1")
    endif()

    set_property(GLOBAL APPEND PROPERTY opentrack-all-modules "${n}")

    foreach(i ${opentrack-all-translations})
        set(t "${CMAKE_CURRENT_SOURCE_DIR}/lang/${i}.ts")
        add_custom_command(OUTPUT "${t}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/lang"
            COMMAND "${Qt5_DIR}/../../../bin/lupdate" -silent -recursive -no-obsolete -locations relative . -ts "${t}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${${n}-c} ${${n}-ui} ${${n}-rc}
            COMMENT "Running lupdate for ${n}/${i}")
        set(target-name "i18n-lang-${i}-module-${n_orig}")
        add_custom_target(${target-name} DEPENDS "${t}")
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-targets-${i}" "${target-name}")
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-files-${i}" "${t}")
    endforeach()

    if(NOT arg_NO-INSTALL)
        set_property(GLOBAL APPEND PROPERTY opentrack-all-source-dirs "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endfunction()

