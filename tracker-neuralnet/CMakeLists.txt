include(opentrack-opencv)
set(host-spec "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_SIZEOF_VOID_P}")
if(host-spec MATCHES "^Linux i[3-6]86 4$")
    return()
endif()

find_package(OpenCV QUIET)
find_package(ONNXRuntime QUIET)
find_package(OpenVINO QUIET )
include(CheckIncludeFileCXX)

message("OpenVINO: ${OpenVINO_DIR}")
message(" found: ${OpenVINO_FOUND}")

if(OpenCV_FOUND AND ONNXRuntime_FOUND)
    if(MSVC)
        add_compile_options(-EHsc)
        add_definitions(-D_HAS_EXCEPTIONS=1)
    endif()

    otr_module(tracker-neuralnet)

    target_link_libraries(${self} PRIVATE
        opentrack-cv
        onnxruntime::onnxruntime
        opencv_calib3d
        opencv_imgproc
        opencv_imgcodecs
        opencv_core
        )

    find_file(HAVE_ONNXRUNTIME_DML_EP "dml_provider_factory.h" NO_DEFAULT_PATH PATHS ${ONNXRuntime_INCLUDE_DIR} NO_CACHE)
    find_file(HAVE_ONNXRUNTIME_OPENVINO_EP "openvino_provider_factory.h" NO_DEFAULT_PATH PATHS ${ONNXRuntime_INCLUDE_DIR} NO_CACHE)
    find_file(HAVE_ONNXRUNTIME_DNNL_EP "dnnl_provider_options.h" NO_DEFAULT_PATH PATHS ${ONNXRuntime_INCLUDE_DIR} NO_CACHE)
    if (HAVE_ONNXRUNTIME_OPENVINO_EP)
        target_link_libraries(${self} PRIVATE openvino::runtime)
    endif()
    message("Execution providers:\n DML ${HAVE_ONNXRUNTIME_DML_EP}\n OpenVINO: ${HAVE_ONNXRUNTIME_OPENVINO_EP}")
    message("  was looking for headers in ${ONNXRuntime_INCLUDE_DIR}")

    configure_file(config.h.in config.h @ONLY)

    install(
        FILES "models/head-localizer.onnx" 
              "models/head-pose-0.2-big.onnx"
              "models/head-pose-0.2-small.onnx"
              "models/head-pose-0.3-big-quantized.onnx"
        DESTINATION "${opentrack-libexec}/models"
        PERMISSIONS ${opentrack-perms-file}
        )

    if(WIN32)
        otr_install_lib("${ONNXRuntime_RUNTIME}" ".")
        # TODO install from openvino
        # * openvino.dll
        # * openvino_intel_cpu_plugin.dll
        # * openvino_onnx_frontend.dll
        # install from onnx
        # * onnxruntime_providers_openvino.dll
        # * onnxruntime_providers_shared.dll
    endif()
endif()
