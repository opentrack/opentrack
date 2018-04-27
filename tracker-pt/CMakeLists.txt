set(modules opencv_core opencv_videoio opencv_imgproc)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    otr_module(tracker-pt-base STATIC)
    target_include_directories(opentrack-tracker-pt-base SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(opentrack-tracker-pt-base opentrack-cv ${modules})
endif()
add_subdirectory(module)

