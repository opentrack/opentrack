include(opentrack-opencv)
set(host-spec "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_SIZEOF_VOID_P}")
if(host-spec MATCHES "^Linux i[3-6]86 4$")
    return()
endif()
if(NOT DEFINED opentrack-use-onnxruntime-avx-dispatch)
    set(opentrack-use-onnxruntime-avx-dispatch 0)
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

    if(opentrack-use-onnxruntime-avx-dispatch)
        target_compile_definitions(${self} PRIVATE "OPENTRACK_HAS_ONNXRUNTIME_CPU_DISPATCH")
        add_library(${self}-if INTERFACE)
        target_include_directories(${self}-if INTERFACE $<TARGET_PROPERTY:onnxruntime::onnxruntime,INTERFACE_INCLUDE_DIRECTORIES>)
        target_compile_definitions(${self}-if INTERFACE $<TARGET_PROPERTY:onnxruntime::onnxruntime,INTERFACE_COMPILE_DEFINITIONS>)
        target_compile_options(${self}-if INTERFACE $<TARGET_PROPERTY:onnxruntime::onnxruntime,INTERFACE_COMPILE_OPTIONS>)
        target_link_libraries(${self} PRIVATE ${self}-if)
        #target_link_libraries(${self} PRIVATE onnxruntime::onnxruntime)
        target_compile_definitions(${self} PRIVATE ORT_API_MANUAL_INIT)
    else()
        target_link_libraries(${self} PRIVATE onnxruntime::onnxruntime)
    endif()

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
              "models/head-pose-0.2-big.onnx"
              "models/head-pose-0.2-small.onnx"
              "models/head-pose-0.3-big-quantized.onnx"
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
