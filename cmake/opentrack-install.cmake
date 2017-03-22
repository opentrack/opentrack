set(opentrack-perms_ WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
set(opentrack-perms PERMISSIONS ${opentrack-perms_})

macro(opentrack_inst2 path)
    install(${ARGN} DESTINATION "${path}" ${opentrack-perms})
endmacro()

macro(opentrack_inst_dir path)
    install(
        DIRECTORY ${ARGN} DESTINATION "${path}"
        FILE_PERMISSIONS ${opentrack-perms_}
        DIRECTORY_PERMISSIONS ${opentrack-perms_}
    )
endmacro()

function(install_sources)
    get_property(source-dirs GLOBAL PROPERTY opentrack-all-source-dirs)
    foreach(k ${source-dirs})
        file(RELATIVE_PATH dest "${CMAKE_SOURCE_DIR}" "${k}")
        opentrack_inst_dir("${opentrack-doc-src-pfx}" "${dest}")
    endforeach()
endfunction()

opentrack_inst_dir("${opentrack-doc-pfx}" ${CMAKE_SOURCE_DIR}/3rdparty-notices)
opentrack_inst_dir("${opentrack-doc-pfx}" "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib")
opentrack_inst_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/cmake")
opentrack_inst_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/bin")

if(WIN32)
    opentrack_inst2(. FILES "${CMAKE_SOURCE_DIR}/bin/qt.conf")
    opentrack_inst2(. FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
    opentrack_inst2(${opentrack-hier-pfx} FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
endif()

opentrack_inst2("${opentrack-doc-pfx}" FILES ${CMAKE_SOURCE_DIR}/README.md)

opentrack_inst2("${opentrack-hier-pfx}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll")
opentrack_inst2("${opentrack-hier-pfx}" FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe")

opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/README.md")
opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md")
opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
opentrack_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")

function(merge_translations)
    set(all-deps "")

    install(CODE "file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/i18n\")")

    get_property(modules GLOBAL PROPERTY opentrack-all-modules)

    foreach(i ${opentrack-all-translations})
        get_property(ts GLOBAL PROPERTY "opentrack-ts-${i}")

        set(deps "")

        foreach(k ${modules})
            list(APPEND deps "i18n-module-${k}")
        endforeach()

        set(qm-output "${CMAKE_BINARY_DIR}/${i}.qm")

        if("${ts}")
            add_custom_command(OUTPUT "${qm-output}"
                COMMAND "${Qt5_DIR}/../../../bin/lrelease" -nounfinished -silent ${ts} -qm "${qm-output}"
                DEPENDS ${deps} ${ts}
                COMMENT "Running lrelease for ${i}")
            add_custom_target(i18n-lang-${i} ALL DEPENDS "${qm-output}")
            list(APPEND all-deps "i18n-lang-${i}")
            install(FILES "${qm-output}" DESTINATION "${opentrack-i18n-pfx}" RENAME "${i}.qm" ${opentrack-perms})
        endif()
    endforeach()
    add_custom_target(i18n DEPENDS ${all-deps})
endfunction()

