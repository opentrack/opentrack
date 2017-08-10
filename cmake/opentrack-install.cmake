set(opentrack-perms_ WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
set(opentrack-perms PERMISSIONS ${opentrack-perms_})

macro(otr_inst2 path)
    install(${ARGN} DESTINATION "${path}" ${opentrack-perms})
endmacro()

macro(otr_inst_dir path)
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
        otr_inst_dir("${opentrack-doc-src-pfx}" "${dest}")
    endforeach()
endfunction()

otr_inst_dir("${opentrack-doc-pfx}" ${CMAKE_SOURCE_DIR}/3rdparty-notices)
otr_inst_dir("${opentrack-doc-pfx}" "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib")
otr_inst_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/cmake")
otr_inst_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/bin")

if(WIN32)
    otr_inst2(. FILES "${CMAKE_SOURCE_DIR}/bin/qt.conf")
    otr_inst2(. FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
    otr_inst2(${opentrack-hier-pfx} FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
endif()

otr_inst2("${opentrack-doc-pfx}" FILES ${CMAKE_SOURCE_DIR}/README.md)

otr_inst2("${opentrack-hier-pfx}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll")
otr_inst2("${opentrack-hier-pfx}" FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe")

otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/README.md")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")

set(opentrack_disable-i18n-update FALSE CACHE BOOL "")

function(merge_translations)
    install(CODE "file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/i18n\")")

    set(all-qm-files "")

    foreach(i ${opentrack_all-translations})
        get_property(ts-files GLOBAL PROPERTY "opentrack-ts-files-${i}")

        set(ts-files_ "")

        foreach(k ${ts-files})
            if(EXISTS "${k}" OR NOT opentrack_disable-i18n-update)
                list(APPEND ts-files_ "${k}")
            endif()
        endforeach()

        get_property(lrelease-binary TARGET "${Qt5_LRELEASE_EXECUTABLE}" PROPERTY IMPORTED_LOCATION)

        if(NOT ".${ts-files_}" STREQUAL ".")
            set(qm-output "${CMAKE_CURRENT_BINARY_DIR}/${i}.qm")
            list(APPEND all-qm-files "${qm-output}")
            add_custom_command(OUTPUT "${qm-output}"
                COMMAND "${lrelease-binary}" -nounfinished -silent ${ts-files_} -qm "${qm-output}"
                DEPENDS ${ts-files}
                COMMENT "Running lrelease for ${i}")
            set(lang-target "i18n-lang-${i}")
            add_custom_target("${lang-target}" DEPENDS "${qm-output}")
            install(FILES "${qm-output}" DESTINATION "${opentrack-i18n-pfx}" RENAME "${i}.qm" ${opentrack-perms})
        endif()
    endforeach()
    add_custom_target(i18n ALL DEPENDS ${all-qm-files})
endfunction()

