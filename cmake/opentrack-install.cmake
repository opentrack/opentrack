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

function(opentrack_install_sources n)
    opentrack_sources(${n} sources)
    file(RELATIVE_PATH subdir "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (i ${sources})
        opentrack_inst2("${opentrack-doc-src-pfx}/${subdir}" FILES "${i}")
    endforeach()
    opentrack_inst2("${opentrack-doc-src-pfx}/${subdir}" FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
endfunction()
