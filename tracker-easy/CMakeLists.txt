include(opentrack-opencv)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    try_compile(tracker-easy_ocv-check "${CMAKE_CURRENT_BINARY_DIR}"
                SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ocv-check.cxx"
                CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${OpenCV_INCLUDE_DIRS}"
                            "-DCXX_STANDARD=17" "-DCXX_STANDARD_REQUIRED=1"
                OUTPUT_VARIABLE krap)
    if(tracker-easy_ocv-check)
        foreach(k video highgui)
            otr_install_lib("opencv_${k}" "${opentrack-hier-pfx}")
        endforeach()

        otr_module(tracker-easy)
        target_include_directories(${self} SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(${self} opencv_core opencv_imgproc opencv_calib3d opencv_video opencv_highgui opentrack-cv opentrack-video)
    endif()
endif()
