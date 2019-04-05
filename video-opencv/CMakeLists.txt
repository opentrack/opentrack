include(opentrack-opencv)

if(OpenCV_FOUND)
    otr_module(video-opencv)
    target_link_libraries(${self} opencv_core opencv_videoio opentrack-video)
endif()
