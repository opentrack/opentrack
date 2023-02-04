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

function(cleanup_visual_studio_debug)
    otr_escape_string(pfx "${CMAKE_INSTALL_PREFIX}")
    install(CODE "file(REMOVE_RECURSE \"${pfx}/.vs\")")
endfunction()

otr_install_dir("${opentrack-doc}" ${CMAKE_SOURCE_DIR}/3rdparty-notices)
otr_install_dir("${opentrack-doc}" "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib")
otr_install_dir("${opentrack-libexec}" "${CMAKE_SOURCE_DIR}/presets")

if(WIN32)
    otr_install_misc(. FILES "${CMAKE_SOURCE_DIR}/bin/qt.conf")
    otr_install_misc(. FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
    otr_install_misc(${opentrack-libexec} FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config")
endif()

otr_install_misc("${opentrack-doc}" FILES ${CMAKE_SOURCE_DIR}/README.md)

otr_install_misc("${opentrack-doc}" FILES "${CMAKE_SOURCE_DIR}/README.md")
otr_install_misc("${opentrack-doc}" FILES "${CMAKE_SOURCE_DIR}/.github/CONTRIBUTING.md")
otr_install_misc("${opentrack-doc}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
otr_install_misc("${opentrack-doc}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
otr_install_misc("${opentrack-doc}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")

# this must be done last because the files may be in use already
# do it last so in case of file-in-use failure, the rest is installed

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    cleanup_visual_studio_debug()
endif()

# For now copy third party needed files into a seperate direcvtory instead of the plugins directory
if (APPLE)
    set(OSX_POST_INSTALL_DIR "/../thirdparty")
endif()
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll")
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient64.dll")
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe")
