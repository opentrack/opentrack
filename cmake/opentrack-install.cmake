set(opentrack-perms PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/qt.conf" DESTINATION . ${opentrack-perms})
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION . ${opentrack-perms})
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION ${opentrack-hier-pfx} ${opentrack-perms})
endif()

install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION ${opentrack-doc-pfx})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty-notices DESTINATION ${opentrack-doc-pfx})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib" DESTINATION ${opentrack-doc-pfx})

install(FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll" DESTINATION ${opentrack-hier-pfx} ${opentrack-perms})
install(FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe"
    DESTINATION ${opentrack-hier-pfx}  ${opentrack-perms})

install(FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt" DESTINATION "${opentrack-doc-src-pfx}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/cmake" DESTINATION "${opentrack-doc-src-pfx}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin" DESTINATION "${opentrack-doc-src-pfx}")
install(FILES "${CMAKE_SOURCE_DIR}/README.md" DESTINATION "${opentrack-doc-src-pfx}")
install(FILES "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md" DESTINATION "${opentrack-doc-src-pfx}")

function(opentrack_install_sources n)
    opentrack_sources(${n} sources)
    file(RELATIVE_PATH subdir "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (i ${sources})
        install(FILES "${i}" DESTINATION "${opentrack-doc-src-pfx}/${subdir}")
    endforeach()
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt" DESTINATION "${opentrack-doc-src-pfx}/${subdir}")
endfunction()
