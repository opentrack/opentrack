find_package(OpenCV QUIET)
otr_module(video STATIC)
if(OpenCV_FOUND)
    target_compile_definitions(${self} PUBLIC -DOTR_VIDEO_HAS_OPENCV)
    target_link_libraries(${self} opencv_videoio opencv_core)
endif()

