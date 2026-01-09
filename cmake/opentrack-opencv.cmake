include_guard(GLOBAL)

include(opentrack-boilerplate)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    foreach(i opencv_calib3d opencv_core opencv_highgui opencv_imgcodecs
            opencv_imgproc opencv_video opencv_videoio)
        otr_install_pdb_for_imported_target(${i})
    endforeach()
endif()
