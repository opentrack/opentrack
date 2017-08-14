set(SDK_REALSENSE "$ENV{RSSDK_DIR}" CACHE PATH "Intel RealSense SDK dir")

if(WIN32 AND SDK_REALSENSE)
    if(CMAKE_COMPILER_IS_GNUCC)
        add_definitions(-fpermissive -Wno-error) # for SDK headers
    endif()
    otr_module(tracker-rs)
    install(FILES "${CMAKE_SOURCE_DIR}/tracker-rs/redist/intel_rs_sdk_runtime_websetup_10.0.26.0396.exe" DESTINATION ${opentrack-contrib-pfx} PERMISSIONS ${opentrack-perms-exec})
    install(FILES "${CMAKE_SOURCE_DIR}/tracker-rs/redist/RS_EULA.rtf" DESTINATION ${opentrack-contrib-pfx} PERMISSIONS ${opentrack-perms-file})
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/rs_impl")
    target_link_libraries(opentrack-tracker-rs opentrack-tracker-rs-impl)
endif()
