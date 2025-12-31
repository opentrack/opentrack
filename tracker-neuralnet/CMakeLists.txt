include(opentrack-opencv)
set(host-spec "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_SIZEOF_VOID_P}")
if(host-spec MATCHES "^Linux i[3-6]86 4$")
    return()
endif()
#if(NOT DEFINED opentrack-use-onnxruntime-avx-dispatch)
#    set(opentrack-use-onnxruntime-avx-dispatch 0)
#endif()

find_package(OpenCV QUIET)
find_package(OpenMP QUIET) # Used to control number of onnx threads.
find_package(ONNXRuntime QUIET)

if(OpenCV_FOUND AND ONNXRuntime_FOUND AND OpenMP_FOUND)
    if(MSVC)
        add_compile_options(-EHsc)
        add_definitions(-D_HAS_EXCEPTIONS=1)
    endif()

    otr_module(tracker-neuralnet)

    target_link_libraries(${self} PRIVATE onnxruntime::onnxruntime)
    target_compile_definitions(${self} PRIVATE ORT_API_MANUAL_INIT)
    otr_install_pdb_for_imported_target(onnxruntime::onnxruntime)

    target_link_libraries(${self} PRIVATE
        opentrack-cv
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
              "models/head-pose-0.4-big-f32.onnx"
              "models/head-pose-0.4-big-int8.onnx"
              "models/head-pose-0.4-small-f32.onnx"
              "models/head-pose-0.4-small-int8.onnx"
              "models/head-pose-0.2-big.onnx"
              "models/head-pose-0.2-small.onnx"
        DESTINATION "${opentrack-libexec}/models"
        PERMISSIONS ${opentrack-perms-file}
        )

    if(WIN32)
        if(NOT opentrack-use-onnxruntime-avx-dispatch)
            otr_install_lib("${ONNXRuntime_RUNTIME}" ".")
        endif()
    endif()
    if(MSVC)
        otr_install_lib("redist/vcomp140.dll" "${opentrack-bin}")
    endif()
endif()
