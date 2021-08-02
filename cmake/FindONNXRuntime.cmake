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
#   ONNXRuntime_FOUND           True if headers and requested libraries were found
#   ONNXRuntime_LIBRARIES       Component libraries to be linked.
#   ONNXRuntime_INCLUDE_DIRS    Include directories.

find_library(ORT_LIB onnxruntime
    CMAKE_FIND_ROOT_PATH_BOTH)

find_path(ORT_INCLUDE onnxruntime_cxx_api.h
    PATH_SUFFIXES onnxruntime/core/session
    CMAKE_FIND_ROOT_PATH_BOTH)

if(ORT_LIB AND ORT_INCLUDE)
    set(ONNXRuntime_FOUND TRUE)
    set(ONNXRuntime_INCLUDE_DIRS "${ORT_INCLUDE}")

    if(NOT TARGET onnxruntime)
        add_library(onnxruntime UNKNOWN IMPORTED)
        set_target_properties(onnxruntime PROPERTIES
            IMPORTED_LOCATION "${ORT_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${ORT_INCLUDE}"
            INTERFACE_LINK_LIBRARIES "onnxruntime")
        list(APPEND ONNXRuntime_LIBRARIES onnxruntime)
    endif()
endif()

unset(ORT_LIB CACHE)
unset(ORT_INCLUDE CACHE)
