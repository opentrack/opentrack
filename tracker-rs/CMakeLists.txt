set(SDK_REALSENSE "$ENV{RSSDK_DIR}" CACHE PATH "Intel RealSense SDK dir")

if(WIN32 AND SDK_REALSENSE AND opentrack-intel)
    if(CMAKE_COMPILER_IS_GNUCC)
        add_definitions(-fpermissive -Wno-error) # for SDK headers
    endif()
    otr_module(tracker-rs)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/rs_impl")
    target_link_libraries(opentrack-tracker-rs PRIVATE opentrack-tracker-rs-impl)
endif()
