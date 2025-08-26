# Guide for building the neuralnet tracker with support for openvino and directml on windows

Opentrack will be build without special settings in release mode. I.e. apart from dependencies cmake settings should remain defaults.

## DirectML

It is already part of windows and the onnx build system will download everything that might be missing. This execution provider should allow running the AI models on most GPUs.

## OpenVINO

The official prebuild binaries can be used. Download openvino_toolkit_windows_<2025 version>_x86_64.zip and unzip it. This execution provider supports execution on CPU and Intel GPUs.

## ONNX Runtime

Needs to be built from sources in order to support both DirectML and OpenVINO. Minimum version is 1.22.

In order to build, execute `build.bat` as follows:
```
./build.bat --config RelWithDebInfo --use_dml --use_openvino CPU --build_shared_lib --parallel <num threads> --compile_no_warning_as_error --skip_tests --cmake_extra_defines CMAKE_INSTALL_PREFIX="<desired install dir>" FETCHCONTENT_TRY_FIND_PACKAGE_MODE=NEVER OpenVINO_DIR="<openvino install dir>\runtime\cmake"

# And then
cmake --install build\Windows\RelWithDebInfo --config RelWithDebInfo
```

This should place all required files in the directory specified by CMAKE_INSTALL_PREFIX.

Python is required for this and it should be a clean environment.

On Linux `--use_dml` must be left out because there is no DirectML on Linux.

# OpenTrack

Cmake variables must be pointed to the dependences.
* ONNXRuntime_DIR: to the runtime install dir. Depending on things, manual cleanup of the onnx build is required. Move .dll to .lib files to `<onnx install dir>/lib` and headers to `<onnx install dir>/include`. Point `ONNXRuntime_DIR` to `<onnx install dir>`
* OpenCV_DIR: to the location of `OpenCVConfig.cmake`
* Qt6_DIR: to the location of `Qt6Config.cmake`
* OpenVINO_DIR: to the location of `OpenVINOConfig.cmake`