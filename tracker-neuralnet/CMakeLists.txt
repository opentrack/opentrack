include(opentrack-opencv)
set(host-spec "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_SIZEOF_VOID_P}")
if(host-spec MATCHES "^Linux i[3-6]86 4$")
    return()
endif()

find_package(OpenCV QUIET)
find_package(OpenMP QUIET) # Used to control number of onnx threads.
find_package(ONNXRuntime QUIET)

if(OpenCV_FOUND AND ONNXRuntime_FOUND AND OpenMP_FOUND)
    if(MSVC)
        add_compile_options(-EHsc)
        add_definitions(-D_HAS_EXCEPTIONS=1)
    endif()

    otr_module(tracker-neuralnet)

    target_link_libraries(${self}
        opentrack-cv
        onnxruntime::onnxruntime
        opencv_calib3d
        opencv_imgproc
        opencv_imgcodecs
        opencv_core
        OpenMP::OpenMP_CXX
        )

    # OpenMP::OpenMP_CXX doesn't set up the -fopenmp linking option, so set it up ourselves.
    if(NOT MSVC)
        target_link_options(${self} PUBLIC ${OpenMP_CXX_FLAGS})
    endif()

    install(
        FILES "models/head-localizer.onnx" 
              "models/head-pose.onnx"
        DESTINATION "${opentrack-libexec}/models"
        PERMISSIONS ${opentrack-perms-file}
        )

    if(WIN32)
        otr_install_lib("${ONNXRuntime_RUNTIME}" ".")
    endif()
    if(MSVC)
        otr_install_lib("redist/vcomp140.dll" "${opentrack-bin}")
    endif()
endif()
