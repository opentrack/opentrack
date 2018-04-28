find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Aruco paper marker tracker static library path")
    if(SDK_ARUCO_LIBPATH)
        set(modules "${SDK_ARUCO_LIBPATH}" opencv_core opencv_calib3d opencv_imgproc opencv_videoio)

        file(TIMESTAMP "${SDK_ARUCO_LIBPATH}" tm "%s" UTC)
        set(cache-str "${tm}|${SDK_ARUCO_LIBPATH}")

        if(NOT aruco_has-working-abi STREQUAL "${cache-str}")
            try_compile(result "${CMAKE_CURRENT_BINARY_DIR}"
                               SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/compile-test/abi.cpp"
                               OUTPUT_VARIABLE krap
                               LINK_LIBRARIES ${modules})
            if(NOT result)
                message(FATAL_ERROR "${krap}\n" "Must use Aruco fork from <https://github.com/opentrack/aruco>")
            endif()

            set(aruco_has-working-abi "${cache-str}" CACHE INTERNAL "" FORCE)
        endif()

        otr_module(tracker-aruco)
        target_link_libraries(opentrack-tracker-aruco opentrack-cv ${modules})
        target_include_directories(opentrack-tracker-aruco SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
    endif()
endif()
