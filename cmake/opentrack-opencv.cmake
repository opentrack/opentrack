include(opentrack-boilerplate)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    # OpenCV 5.0 split the calib3d module into calib, geometry, stereo, and
    # ptcloud, and renamed features2d to features. The opencv_calib3d and
    # opencv_features2d CMake targets no longer exist in OpenCV 5.
    #
    # Detect availability via TARGET checks (the two module sets are mutually
    # exclusive across OpenCV 4 and 5) rather than version-string parsing.
    # On OpenCV 4 these expand to exactly the previous target names, so
    # existing behavior is unchanged.
    if(TARGET opencv_calib3d)
        set(OTR_OPENCV_CALIB_MODULES opencv_calib3d)
        set(OTR_OPENCV_FEATURES_MODULES opencv_features2d)
    elseif(TARGET opencv_calib)
        set(OTR_OPENCV_CALIB_MODULES opencv_calib opencv_geometry)
        set(OTR_OPENCV_FEATURES_MODULES opencv_features)
    else()
        set(OTR_OPENCV_CALIB_MODULES "")
        set(OTR_OPENCV_FEATURES_MODULES "")
    endif()

    foreach(i ${OTR_OPENCV_CALIB_MODULES} opencv_core opencv_highgui opencv_imgcodecs
            opencv_imgproc opencv_video opencv_videoio)
        otr_install_pdb_for_imported_target(${i})
    endforeach()
endif()
