include_guard(GLOBAL)

if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()

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

otr_install_dir("${opentrack-doc}" "3rdparty-notices")
otr_install_dir("${opentrack-doc}" "settings" "${CMAKE_SOURCE_DIR}/contrib")
otr_install_dir("${opentrack-libexec}" "presets")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        otr_install_misc(. FILES "bin/cleye.config")
        otr_install_misc(${opentrack-libexec} FILES "bin/cleye.config")
    endif()
endif()

otr_install_misc("${opentrack-doc}" FILES README.md)

otr_install_misc("${opentrack-doc}" FILES "README.md")
otr_install_misc("${opentrack-doc}" FILES ".github/CONTRIBUTING.md")
otr_install_misc("${opentrack-doc}" FILES "WARRANTY.txt")
otr_install_misc("${opentrack-doc}" FILES "OPENTRACK-LICENSING.txt")
otr_install_misc("${opentrack-doc}" FILES "AUTHORS.md")

# this must be done last because the files may be in use already
# do it last so in case of file-in-use failure, the rest is installed

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    cleanup_visual_studio_debug()
endif()

# For now copy third party needed files into a seperate direcvtory instead of the plugins directory
if (APPLE)
    set(OSX_POST_INSTALL_DIR "/../thirdparty")
endif()
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES "bin/freetrackclient.dll")
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES "bin/freetrackclient64.dll")
otr_install_exec("${opentrack-libexec}${OSX_POST_INSTALL_DIR}" FILES
    "bin/NPClient.dll"
    "bin/NPClient64.dll"
    "bin/TrackIR.exe")
