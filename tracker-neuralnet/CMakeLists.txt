include(opentrack-opencv)
find_package(OpenCV QUIET)
find_package(OpenMP QUIET) # Used to control number of onnx threads.
set(SDK_ONNX_LIBPATH "" CACHE FILEPATH "Full path of onnx library")

if(OpenCV_FOUND AND SDK_ONNX_LIBPATH AND OpenMP_FOUND)
    get_filename_component(ONNX_INCLUDE_DIR "${SDK_ONNX_LIBPATH}" DIRECTORY)
    get_filename_component(ONNX_INCLUDE_DIR "${ONNX_INCLUDE_DIR}" ABSOLUTE)
    set(ONNX_INCLUDE_DIR "${ONNX_INCLUDE_DIR}/../include")

    otr_module(tracker-neuralnet)
    target_include_directories(${self} SYSTEM PUBLIC 
        ${OpenCV_INCLUDE_DIRS} "${ONNX_INCLUDE_DIR}")
    target_link_libraries(${self} 
        opentrack-cv "${SDK_ONNX_LIBPATH}" opencv_imgproc opencv_core 
        opencv_imgcodecs opencv_calib3d
        OpenMP::OpenMP_C)

    install(
        FILES "models/head-localizer.onnx" "models/head-pose.onnx"
        DESTINATION "${opentrack-libexec}/models"
        PERMISSIONS ${opentrack-perms-file})
endif()