include_guard(GLOBAL)

add_custom_target(moc COMMENT "Qt temporary files")

set(opentrack-perms-file WORLD_READ OWNER_WRITE OWNER_READ GROUP_READ)
set(opentrack-perms-dir WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
set(opentrack-perms-exec "${opentrack-perms-dir}")

set(new-hier-path "#pragma once
#ifdef QT_CORE_LIB
#   include <QCoreApplication>
#   include <QString>
#   include \"compat/base-path.hpp\"
#endif
#define OPENTRACK_LIBRARY_PATH \"${opentrack-hier-path}\"
#define OPENTRACK_DOC_PATH \"${opentrack-hier-doc}\"
#define OPENTRACK_CONTRIB_PATH \"${opentrack-hier-doc}contrib/\"
#define OPENTRACK_I18N_PATH \"${opentrack-i18n-path}\"
")

include_directories("${CMAKE_BINARY_DIR}")

set(hier-path-filename "${CMAKE_BINARY_DIR}/__opentrack-library-path.h")
set(orig-hier-path "")
if(EXISTS "${hier-path-filename}")
    file(READ ${hier-path-filename} orig-hier-path)
endif()
if(NOT (orig-hier-path STREQUAL new-hier-path))
    file(WRITE "${hier-path-filename}" "${new-hier-path}")
endif()

function(otr_glob_sources var)
    set(basedir "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach(dir . ${ARGN})
        set(dir "${basedir}/${dir}")
        file(GLOB ${var}-cxx ${dir}/*.cpp)
        file(GLOB ${var}-cc ${dir}/*.c)
        file(GLOB ${var}-hh ${dir}/*.h ${dir}/*.hpp)
        file(GLOB ${var}-res ${dir}/*.rc)
        foreach(f ${var}-res)
            set_source_files_properties(${f} PROPERTIES LANGUAGE RC)
        endforeach()
        file(GLOB ${var}-ui ${dir}/*.ui)
        file(GLOB ${var}-rc ${dir}/*.qrc)
        set(${var}-all ${${var}-cc} ${${var}-cxx} ${${var}-hh} ${${var}-rc} ${${var}-res})
        foreach(i ui rc res cc cxx hh all)
            set(${var}-${i} "${${var}-${i}}" PARENT_SCOPE)
        endforeach()
    endforeach()
endfunction()

function(otr_fixup_subsystem n)
    otr_find_msvc_editbin(editbin-pathname)
    if(MSVC)
        set(subsystem WINDOWS)
        get_property(type TARGET "${n}" PROPERTY TYPE)
        if (NOT type STREQUAL "STATIC_LIBRARY")
            set(loc "$<TARGET_FILE:${n}>")
            add_custom_command(TARGET "${n}"
                               POST_BUILD
                               COMMAND "${editbin-pathname}" -nologo -SUBSYSTEM:${subsystem},5.01 -OSVERSION:5.1 \"${loc}\")
        endif()
    endif()
endfunction()

function(otr_compat target)
    if(NOT MSVC)
        set_property(SOURCE ${${target}-moc} APPEND_STRING PROPERTY COMPILE_FLAGS "-w -Wno-error ")
    endif()

    if(UNIX) # no-op on OSX
        target_link_libraries(${target} m)
    endif()

    get_property(type TARGET "${n}" PROPERTY TYPE)
    if (".${TYPE}" STREQUAL ".EXECUTABLE")
        otr_fixup_subsystem(${target})
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX AND NOT MSVC)
        # gives incorrect result
        #get_property(linker-lang TARGET ${target} PROPERTY LINKER_LANGUAGE)

        #set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS "-fvisibility=hidden ")

        #if (NOT ".${${target}-cxx}" STREQUAL ".") # not a C only target
        #    set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS "-fuse-cxa-atexit ")
        #endif()

        #if(NOT APPLE)
        #    set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--as-needed ")
        #endif()
    endif()
endfunction()

function(otr_is_sln_generator var)
    string(FIND "${CMAKE_GENERATOR}" "Visual Studio " is-msvc)
    if(is-msvc EQUAL 0)
        set(${var} TRUE PARENT_SCOPE)
    else()
        set(${var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(otr_find_msvc_editbin varname)
    if(MSVC)
        get_filename_component(linker-dir "${CMAKE_LINKER}" DIRECTORY)
        set("${varname}" "${linker-dir}/editbin.exe" PARENT_SCOPE)
    else()
        set("${varname}" "editbin" PARENT_SCOPE)
    endif()
endfunction()

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
        "SOURCES;SUBDIRS"
        ${ARGN}
    )

    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "otr_module bad formals: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    set(n "opentrack-${n_}")

    if(NOT arg_SUBDIRS)
        otr_glob_sources(${n} .)
    else()
        otr_glob_sources(${n} ${arg_SUBDIRS})
    endif()

    list(APPEND ${n}-all ${arg_SOURCES})

    if(NOT arg_NO-QT)
        otr_qt(${n})
    else()
        set(arg_NO-COMPAT TRUE)
    endif()

    if(NOT arg_NO-QT)
        set_property(SOURCE ${${n}-moc} ${${n}-uih} PROPERTY GENERATED TRUE)
        add_custom_target(moc-${n} DEPENDS ${${n}-moc} ${${n}-uih} ${${n}-rc} COMMENT "")
        set_property(TARGET moc-${n} PROPERTY GENERATED TRUE)
        set_property(TARGET moc-${n} PROPERTY FOLDER "moc")
        add_dependencies(moc "moc-${n}")
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
        set_property(TARGET "${n}" PROPERTY PREFIX "")
    endif()

    if(NOT arg_NO-QT)
        otr_qt2("${n}")
    endif()

    set(self "${n}" PARENT_SCOPE)

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

    string(REPLACE "-" "_" build-n ${n_})
    string(TOUPPER "${build-n}" build-n)
    set_property(TARGET ${n} PROPERTY DEFINE_SYMBOL "BUILD_${build-n}")

    get_property(ident GLOBAL PROPERTY opentrack-ident)
    if (".${ident}" STREQUAL ".")
        message(FATAL_ERROR "must set global property `opentrack-ident' in `opentrack-variant.cmake'")
    endif()
    set_property(TARGET ${n} APPEND PROPERTY COMPILE_DEFINITIONS "OPENTRACK_ORG=\"${ident}\"")

    if(arg_STATIC)
        set(arg_NO-INSTALL TRUE)
    endif()

    otr_compat(${n})

    if(CMAKE_COMPILER_IS_CLANGXX AND (arg_EXECUTABLE OR NOT arg_BIN))
        set(opts
            weak-vtables
            header-hygiene
        )
        foreach(k ${opts})
            set_property(TARGET "${n}" APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-${k} ")
        endforeach()
    endif()

    if(NOT arg_NO-INSTALL)
        if(arg_BIN AND WIN32)
            install(TARGETS "${n}" RUNTIME DESTINATION . PERMISSIONS ${opentrack-perms-exec})
        else()
            install(TARGETS "${n}" ${opentrack-hier-str} PERMISSIONS ${opentrack-perms-exec})
        endif()
        if(MSVC)
            set(opentrack_install-debug-info FALSE CACHE BOOL "Whether to build and install debug info at install time")
            if(opentrack_install-debug-info)
                otr_install_pdb_current_project(${n})
            endif()
        endif()
    endif()

    if(NOT arg_NO-QT)
        otr_i18n_for_target_directory(${n_})
    endif()

    set_property(GLOBAL APPEND PROPERTY opentrack-all-modules "${n_}")
    set_property(GLOBAL APPEND PROPERTY opentrack-all-source-dirs "${CMAKE_CURRENT_SOURCE_DIR}")

    #make_directory("${CMAKE_CURRENT_BINARY_DIR}")
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
