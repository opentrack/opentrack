# FindONNXRuntime
# ===============
#
# Find an ONNX Runtime installation.
# ONNX Runtime is a cross-platform inference and training machine-learning
# accelerator.
#
# Input variables
# ---------------
# 
#   ONNXRuntime_ROOT            Set root installation.
#
# Output variable
# ---------------
# 
#   ONNXRuntime_FOUND           Variable indicating that ONNXRuntime has been
#                               found.
#   ONNXRuntime_LIBRARIES       Library implementing ONNXRuntime
#   ONNXRuntime_INCLUDE_DIRS    Headers for ONNXRuntime

find_library(ORT_LIB onnxruntime
    CMAKE_FIND_ROOT_PATH_BOTH)
find_path(ORT_INCLUDE onnxruntime/core/session/onnxruntime_cxx_api.h
    CMAKE_FIND_ROOT_PATH_BOTH)

if(ORT_LIB AND ORT_INCLUDE)
    set(ONNXRuntime_FOUND TRUE)
    # For CMake output only
    set(ONNXRuntime_LIBRARIES "${ORT_LIB}" CACHE STRING "ONNX Runtime libraries")
    set(ONNXRuntime_INCLUDE_DIRS "${ORT_INCLUDE}" CACHE STRING "ONNX Runtime include path")
endif()
