include_guard(GLOBAL)

if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()

set(opentrack-perms-file WORLD_READ OWNER_WRITE OWNER_READ GROUP_READ)
set(opentrack-perms-dir WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
set(opentrack-perms-exec "${opentrack-perms-dir}")

set(new-hier-path "#pragma once
#ifdef QT_CORE_LIB
#   include \"compat/base-path.hpp\"
#endif

#if defined __APPLE__
#   define OPENTRACK_LIBRARY_EXTENSION \"dylib\"
#elif defined _WIN32
#   define OPENTRACK_LIBRARY_EXTENSION \"dll\"
#else
#   define OPENTRACK_LIBRARY_EXTENSION \"so\"
#endif

#define OPENTRACK_LIBRARY_PREFIX \"\"
#define OPENTRACK_LIBRARY_PATH \"${opentrack-runtime-libexec}\"
#define OPENTRACK_DOC_PATH \"${opentrack-runtime-doc}\"
#define OPENTRACK_CONTRIB_PATH \"${opentrack-runtime-doc}contrib/\"
#define OPENTRACK_I18N_PATH \"${opentrack-runtime-i18n}\"
")

function(otr_write_library_paths)
    set(hier-path-filename "${CMAKE_BINARY_DIR}/opentrack-library-path.hxx")
    set(orig-hier-path "")
    if(EXISTS "${hier-path-filename}")
        file(READ ${hier-path-filename} orig-hier-path)
    endif()
    if(NOT (orig-hier-path STREQUAL new-hier-path))
        file(WRITE "${hier-path-filename}" "${new-hier-path}")
    endif()
endfunction()

otr_write_library_paths()

function(otr_glob_sources var)
    set(basedir "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach(i ui rc res cc cxx hh all)
        set(${var}-${i} "")
    endforeach()
    foreach(dir ${ARGN})
        set(dir "${basedir}/${dir}")
        file(GLOB cxx CONFIGURE_DEPENDS "${dir}/*.cpp")
        file(GLOB cc CONFIGURE_DEPENDS "${dir}/*.c")
        file(GLOB hh CONFIGURE_DEPENDS "${dir}/*.h" "${dir}/*.hpp" "${dir}/*.inc")
        file(GLOB res CONFIGURE_DEPENDS "${dir}/*.rc")
        foreach(f res)
            set_source_files_properties(${f} PROPERTIES LANGUAGE RC)
        endforeach()
        file(GLOB ui CONFIGURE_DEPENDS "${dir}/*.ui")
        file(GLOB rc CONFIGURE_DEPENDS "${dir}/*.qrc")
        set(all ${cc} ${cxx} ${hh} ${res})
        foreach(i ui rc res cc cxx hh all)
            set(${var}-${i} "${${var}-${i}}" ${${i}} PARENT_SCOPE)
        endforeach()
    endforeach()
endfunction()

#function(otr_find_msvc_editbin varname)
#    if(MSVC)
#        get_filename_component(linker-dir "${CMAKE_LINKER}" DIRECTORY)
#        set("${varname}" "${linker-dir}/editbin.exe" PARENT_SCOPE)
#    else()
#        set("${varname}" "editbin" PARENT_SCOPE)
#    endif()
#endfunction()

#function(otr_fixup_subsystem n)
#    if(MSVC)
#        otr_find_msvc_editbin(pathname)
#        get_property(type TARGET "${n}" PROPERTY TYPE)
#        if(NOT type STREQUAL "STATIC_LIBRARY")
#            set(loc "$<TARGET_FILE:${n}>")
#            if(CMAKE_SIZEOF_VOID_P GREATER 4)
#                set(s WINDOWS,5.02)
#            else()
#                set(s WINDOWS,5.01)
#            endif()
#            add_custom_command(TARGET "${n}"
#                               POST_BUILD
#                               COMMAND "${pathname}" -nologo -SUBSYSTEM:${s} -OSVERSION:5.1 \"${loc}\")
#        endif()
#    endif()
#endfunction()

function(otr_compat target)
    if(NOT MSVC)
        set_property(SOURCE ${${target}-moc} APPEND PROPERTY COMPILE_OPTIONS "-w;-Wno-error")
    endif()

    if(UNIX) # no-op on OSX
        target_link_libraries(${target} PRIVATE m)
    endif()

    get_property(type TARGET "${n}" PROPERTY TYPE)
    #if (type STREQUAL "EXECUTABLE")
    #    otr_fixup_subsystem(${target})
    #endif()
endfunction()

function(otr_is_sln_generator var)
    string(FIND "${CMAKE_GENERATOR}" "Visual Studio " is-msvc)
    if(is-msvc EQUAL 0)
        set(${var} TRUE PARENT_SCOPE)
    else()
        set(${var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(otr_install_pdb_current_project target)
    if(MSVC)
        install(FILES "$<TARGET_PDB_FILE:${target}>" DESTINATION "${opentrack-debug}" PERMISSIONS ${opentrack-perms-file})
    endif()
endfunction()

function(otr_module n_)
    message(STATUS "module ${n_}")
    cmake_parse_arguments(arg
        "STATIC;NO-COMPAT;BIN;EXECUTABLE;NO-QT;NO-I18N;WIN32-CONSOLE;NO-INSTALL;RELINK"
        "LINK;COMPILE"
        "SOURCES;SUBDIRS"
        ${ARGN}
    )

    if(NOT "${arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "otr_module(${n_}) bad formals: ${arg_UNPARSED_ARGUMENTS}")
    endif()

    set(n "opentrack-${n_}")

    if(NOT arg_SUBDIRS)
        otr_glob_sources(${n} .)
    else()
        otr_glob_sources(${n} . ${arg_SUBDIRS})
    endif()

    list(APPEND ${n}-all ${arg_SOURCES})

    if(NOT arg_NO-QT)
        otr_qt(${n})
    else()
        set(arg_NO-COMPAT TRUE)
        set(arg_NO-I18N TRUE)
    endif()

    if(arg_EXECUTABLE)
        if (APPLE)
            set(subsys "MACOSX_BUNDLE")
        elseif(NOT WIN32)
            set(subsys "")
        elseif(arg_WIN32-CONSOLE)
            set(subsys "")
        else()
            set(subsys "WIN32")
        endif()

        add_executable(${n} ${subsys} "${${n}-all}")
        set_target_properties(${n} PROPERTIES
                              SUFFIX "${opentrack-binary-suffix}"
                              OUTPUT_NAME "${n_}"
                              PREFIX "")
        if(MSVC)
            target_compile_options(${n} PRIVATE -GA)
        endif()
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

    if(NOT arg_RELINK AND FALSE)
        set_property(TARGET ${n} PROPERTY LINK_DEPENDS_NO_SHARED TRUE)
    else()
        set_property(TARGET ${n} PROPERTY LINK_DEPENDS_NO_SHARED FALSE)
    endif()

    if(NOT arg_NO-QT)
        target_link_libraries(${n} PRIVATE ${qt-imported-targets})
    endif()

    if(NOT arg_NO-COMPAT)
        target_link_libraries(${n} PRIVATE opentrack-api opentrack-options opentrack-compat)
    endif()

    string(REPLACE "-" "_" build-n ${n_})
    string(TOUPPER "${build-n}" build-n)
    target_compile_definitions(${n} PRIVATE "BUILD_${build-n}")
    include(opentrack-org)

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
            target_compile_options(${n} PRIVATE "-Wno-${k}")
        endforeach()
    endif()

    if(NOT arg_NO-INSTALL)
        # Librarys/executable
        if(arg_BIN)
            if (APPLE)
                install(TARGETS "${n}"
                RUNTIME DESTINATION "${opentrack-bin}"
                BUNDLE	DESTINATION "${opentrack-bin}"
                LIBRARY DESTINATION "${opentrack-bin}/Library"
                RESOURCE DESTINATION "${opentrack-bin}/opentrack.app/Resource"
                PERMISSIONS ${opentrack-perms-exec})
            else()
                install(TARGETS "${n}"
                    RUNTIME DESTINATION "${opentrack-bin}"
                    LIBRARY DESTINATION "${opentrack-libexec}"
                    PERMISSIONS ${opentrack-perms-exec})
            endif()
        else()
            # Plugins
            install(TARGETS "${n}"
                    RUNTIME DESTINATION "${opentrack-libexec}"
                    LIBRARY DESTINATION "${opentrack-libexec}"
                    PERMISSIONS ${opentrack-perms-exec})
        endif()

        if(MSVC)
            set(opentrack_install-debug-info FALSE CACHE BOOL "Whether to build and install debug info at install time")
            if(opentrack_install-debug-info)
                otr_install_pdb_current_project(${n})
            endif()
        endif()
    endif()

    if(NOT arg_NO-I18N)
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
    file(GLOB projects CONFIGURE_DEPENDS ${globs})
    list(SORT projects)
    set("${var}" "${projects}" PARENT_SCOPE)
endfunction()

function(otr_pdb_for_dll varname path)
    set("${varname}" "" PARENT_SCOPE)
    get_filename_component(dir "${path}" DIRECTORY)
    get_filename_component(name-only "${path}" NAME_WE)
    set(pdb-path "${dir}/${name-only}.pdb")
    if(EXISTS "${pdb-path}")
        set("${varname}" "${pdb-path}" PARENT_SCOPE)
    endif()
endfunction()

function(otr_install_lib target dest)
    if(WIN32)
        string(FIND "${target}" "/" idx)
        if(idx EQUAL -1)
            string(TOUPPER "${CMAKE_BUILD_TYPE}" _bt)
            set(sources "LOCATION_${_bt};LOCATION;IMPORTED_LOCATION_${_bt};IMPORTED_LOCATION")

            foreach(prop IN LISTS sources)
                set(path "")
                get_property(path TARGET "${target}" PROPERTY "${prop}")
                if(path)
                    break()
                endif()
            endforeach()
        else()
            set(path "${target}")
        endif()
        if("${path}" STREQUAL "")
            message(FATAL_ERROR "Can't find ${target}")
        endif()
        string(TOLOWER "${path}" path_)
        if(NOT path_ MATCHES "\\.(a|lib)$")
            if(MSVC AND opentrack_install-debug-info)
                set(pdb-path "")
                otr_pdb_for_dll(pdb-path "${path}")
                if(pdb-path)
                    install(FILES "${pdb-path}" DESTINATION "${opentrack-debug}" PERMISSIONS ${opentrack-perms-exec})
                endif()
            endif()
            install(FILES "${path}" DESTINATION "${dest}" PERMISSIONS ${opentrack-perms-exec})
        endif()
    endif()
endfunction()

function(otr_install_pdb_for_imported_target target)
    if(MSVC)
        set(PDB_PATH "$<PATH:REPLACE_EXTENSION,$<TARGET_FILE:${target}>,.pdb>")
        install(FILES "${PDB_PATH}" DESTINATION "${opentrack-debug}" OPTIONAL)
    endif()
endfunction()
