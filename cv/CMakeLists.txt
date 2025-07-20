include(opentrack-opencv)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    try_compile(cv_use-ipp "${CMAKE_CURRENT_BINARY_DIR}"
                SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ocv-check.cxx"
                CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${OpenCV_INCLUDE_DIRS}"
                            "-DCXX_STANDARD=20" "-DCXX_STANDARD_REQUIRED=1"
                OUTPUT_VARIABLE krap)
    otr_module(cv STATIC)
    target_link_libraries(${self} PRIVATE opencv_core opentrack-video)
    target_include_directories(${self} SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
    if(cv_use-ipp)
        target_compile_definitions(${self} PUBLIC -DOTR_HAS_CV_IPP)
    endif()
endif()
