macro(otr_inst2 path)
    install(${ARGN} DESTINATION "${path}" PERMISSIONS ${opentrack-perms-file})
endmacro()

macro(otr_inst_exec path)
    install(${ARGN} DESTINATION "${path}" PERMISSIONS ${opentrack-perms-file})
endmacro()

macro(otr_inst_dir path)
    install(
        DIRECTORY ${ARGN} DESTINATION "${path}"
        FILE_PERMISSIONS ${opentrack-perms-file}
        DIRECTORY_PERMISSIONS ${opentrack-perms-dir}
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

otr_inst_exec("${opentrack-hier-pfx}" FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll")
otr_inst_exec("${opentrack-hier-pfx}" FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe")

if(WIN32)
    otr_inst_exec("${opentrack-hier-pfx}" FILES "${CMAKE_SOURCE_DIR}/bin/amcap.exe")
endif()

otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/README.md")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/WARRANTY.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/OPENTRACK-LICENSING.txt")
otr_inst2("${opentrack-doc-src-pfx}" FILES "${CMAKE_SOURCE_DIR}/AUTHORS.md")

set(opentrack_disable-i18n-update FALSE CACHE BOOL "")

