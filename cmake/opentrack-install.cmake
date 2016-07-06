include(opentrack-hier)

set(opentrack-perms PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
endif()

install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION ${opentrack-hier-doc})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty-notices DESTINATION ${opentrack-hier-doc})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/settings" "${CMAKE_SOURCE_DIR}/contrib" DESTINATION ${opentrack-hier-doc})

install(FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll" DESTINATION ${opentrack-hier-pfx} ${opentrack-perms})
install(FILES
    "${CMAKE_SOURCE_DIR}/bin/NPClient.dll"
    "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll"
    "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe"
    DESTINATION ${opentrack-hier-pfx}  ${opentrack-perms})
