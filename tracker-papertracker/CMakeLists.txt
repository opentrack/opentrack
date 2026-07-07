include(opentrack-opencv)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    set(opencv-modules core imgproc flann)
    foreach(k ${opencv-modules})
        if(NOT TARGET "opencv_${k}")
            return()
        endif()
    endforeach()
    foreach(k ${OTR_OPENCV_CALIB_MODULES} ${OTR_OPENCV_FEATURES_MODULES})
        if(NOT TARGET "${k}")
            return()
        endif()
    endforeach()
    foreach(k ${opencv-modules})
        otr_install_lib("opencv_${k}" "${opentrack-libexec}")
    endforeach()
    foreach(k ${OTR_OPENCV_CALIB_MODULES} ${OTR_OPENCV_FEATURES_MODULES})
        otr_install_lib("${k}" "${opentrack-libexec}")
    endforeach()

    set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Aruco paper marker tracker static library path")
    if(SDK_ARUCO_LIBPATH)
        set(modules "${SDK_ARUCO_LIBPATH}" ${OTR_OPENCV_CALIB_MODULES} opencv_imgproc opencv_core)

        get_filename_component(dir "${SDK_ARUCO_LIBPATH}" DIRECTORY)
        get_filename_component(dir "${dir}" ABSOLUTE)
        set(dir "${dir}/include")

        try_compile(tracker-aruco_has-working-abi "${CMAKE_CURRENT_BINARY_DIR}"
                           SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/compile-test/abi.cpp"
                           CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${dir}"
                                       "-DCXX_STANDARD=20" "-DCXX_STANDARD_REQUIRED=1"
                           OUTPUT_VARIABLE krap)
        if(NOT tracker-aruco_has-working-abi)
            message(FATAL_ERROR "${krap}\n" "Must use Aruco fork from <https://github.com/opentrack/aruco>")
        endif()

        maybe_add_static_define()
        otr_install_lib("${SDK_ARUCO_LIBPATH}" "${opentrack-libexec}")

        otr_module(tracker-papertracker)
        target_include_directories(${self} SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS} "${dir}")
        target_link_libraries(${self} PRIVATE opentrack-cv ${modules})
    endif()
endif()