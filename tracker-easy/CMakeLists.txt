include(opentrack-opencv)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    try_compile(tracker-easy_ocv-check "${CMAKE_CURRENT_BINARY_DIR}"
                SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ocv-check.cxx"
                CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${OpenCV_INCLUDE_DIRS}"
                            "-DCXX_STANDARD=20" "-DCXX_STANDARD_REQUIRED=1"
                OUTPUT_VARIABLE krap)
    if(tracker-easy_ocv-check)
        foreach(k highgui videoio imgcodecs imgproc calib3d video features2d flann)
            otr_install_lib("opencv_${k}" "${opentrack-libexec}")
        endforeach()

        if(CMAKE_COMPILER_IS_GNUCXX)
            add_compile_options(-Wno-sign-compare)
        elseif(MSVC)
            add_compile_options(-wd4018)
        endif()

        otr_module(tracker-easy)
        target_include_directories(${self} SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(${self} PRIVATE opencv_core opencv_imgproc opencv_calib3d opencv_video opencv_highgui opentrack-cv opentrack-video)
    endif()
endif()
