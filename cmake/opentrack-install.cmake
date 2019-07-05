include_guard(GLOBAL)
macro(otr_install_misc path)
    install(${ARGN} DESTINATION "${path}" PERMISSIONS ${opentrack-perms-file})
endmacro()

macro(otr_install_exec path)
    install(${ARGN} DESTINATION "${path}" PERMISSIONS ${opentrack-perms-file})
endmacro()

macro(otr_install_dir path)
    install(
        DIRECTORY ${ARGN} DESTINATION "${path}"
        FILE_PERMISSIONS ${opentrack-perms-file}
        DIRECTORY_PERMISSIONS ${opentrack-perms-dir}
        PATTERN ".gtm" EXCLUDE
    )
endmacro()

function(otr_setup_refresh_install_dir)
    if((NOT CMAKE_INSTALL_PREFIX STREQUAL "") AND (NOT opentrack-doc-src-pfx STREQUAL ""))
        otr_escape_string(dir "${CMAKE_INSTALL_PREFIX}/${opentrack-doc-src-pfx}/")
        install(CODE "file(REMOVE_RECURSE \"${dir}\")")
    endif()
endfunction()

function(otr_install_sources)
    otr_setup_refresh_install_dir()
    get_property(source-dirs GLOBAL PROPERTY opentrack-all-source-dirs)
    foreach(k ${source-dirs})
        file(RELATIVE_PATH dest "${CMAKE_SOURCE_DIR}" "${k}")
        otr_install_dir("${opentrack-doc-src-pfx}" "${dest}")
    endforeach()

    otr_install_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/cmake")
    otr_install_dir("${opentrack-doc-src-pfx}" "${CMAKE_SOURCE_DIR}/bin")

    otr_install_misc("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
    otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/README.md")
    otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/.github/CONTRIBUTING.md")
    otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
    otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
    otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")
endfunction()

function(cleanup_visual_studio_debug)
    otr_escape_string(pfx "${CMAKE_INSTALL_PREFIX}")
    install(CODE "file(REMOVE_RECURSE \"${pfx}/.vs\")")
endfunction()

otr_install_dir("${opentrack-doc-pfx}" ${CMAKE_SOURCE_DIR}/3rdparty-notices)
otr_install_dir("${opentrack-doc-pfx}" "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib")

if(WIN32)
    otr_install_misc(. FILES "${CMAKE_SOURCE_DIR}/bin/qt.conf")
    otr_install_misc(. FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
    otr_install_misc(${opentrack-hier-pfx} FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
endif()

otr_install_misc("${opentrack-doc-pfx}" FILES ${CMAKE_SOURCE_DIR}/README.md)

otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/README.md")
otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/.github/CONTRIBUTING.md")
otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
otr_install_misc("${opentrack-doc-pfx}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")

# this must be done last because the files may be in use already
# do it last so in case of file-in-use failure, the rest is installed

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    cleanup_visual_studio_debug()
endif()

otr_install_exec("${opentrack-hier-pfx}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll")
otr_install_exec("${opentrack-hier-pfx}" FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe")
