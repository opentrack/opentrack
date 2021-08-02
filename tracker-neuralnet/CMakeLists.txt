include(opentrack-opencv)
find_package(OpenCV QUIET COMPONENTS imgproc core imgcodecs calib3d)
find_package(OpenMP QUIET) # Used to control number of onnx threads.
find_package(ONNXRuntime QUIET)

if(OpenCV_FOUND AND ONNXRuntime_FOUND AND OpenMP_FOUND)
    otr_module(tracker-neuralnet)
    target_link_libraries(${self} 
        opentrack-cv
        onnxruntime
        ${OpenCV_LIBS} 
        OpenMP::OpenMP_C
        )
    install(
        FILES "models/head-localizer.onnx" 
              "models/head-pose.onnx"
        DESTINATION "${opentrack-libexec}/models"
        PERMISSIONS ${opentrack-perms-file}
        )
endif()
