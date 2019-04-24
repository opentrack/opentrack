function(maybe_add_static_define)
    get_filename_component(aruco-ext "${SDK_ARUCO_LIBPATH}" EXT)
    string(TOLOWER "${aruco-ext}" aruco-ext)
    if(aruco-ext STREQUAL ".lib" OR aruco-ext STREQUAL ".a")
        add_definitions(-DARUCO_STATIC)
    endif()
endfunction()


include(opentrack-opencv)
find_package(OpenCV QUIET)

if(OpenCV_FOUND)
    set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Aruco paper marker tracker static library path")
    if(SDK_ARUCO_LIBPATH)
        set(modules "${SDK_ARUCO_LIBPATH}" opencv_calib3d opencv_imgproc opencv_core)

        get_filename_component(dir "${SDK_ARUCO_LIBPATH}" DIRECTORY)
        get_filename_component(dir "${dir}" ABSOLUTE)
        set(dir "${dir}/include")

        try_compile(tracker-aruco_has-working-abi "${CMAKE_CURRENT_BINARY_DIR}"
                           SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/compile-test/abi.cpp"
                           CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${dir}"
                                       "-DCXX_STANDARD=17" "-DCXX_STANDARD_REQUIRED=1"
                           OUTPUT_VARIABLE krap)
        if(NOT tracker-aruco_has-working-abi)
            message(FATAL_ERROR "${krap}\n" "Must use Aruco fork from <https://github.com/opentrack/aruco>")
        endif()

        maybe_add_static_define()
        otr_install_lib("${SDK_ARUCO_LIBPATH}" "${opentrack-hier-pfx}")

        otr_module(tracker-aruco)
        target_include_directories(${self} SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS} "${dir}")
        target_link_libraries(${self} opentrack-cv ${modules})
    endif()
endif()
