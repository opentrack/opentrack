#
# qtbase configure line for safekeeping
#

# "../configure" -prefix d:\dev\qt-5.10.0 -no-ico -no-gif               \
# -opengl desktop -no-fontconfig -no-harfbuzz                           \
# -nomake tests -no-mp -release -opensource -shared -confirm-license    \
# -no-freetype -force-debug-info -separate-debug-info                   \
# -make-tool jom -platform win32-msvc -static-runtime

# remember to change -MD to -MT in mkspecs/
# also add CFLAGS -Zi and LFLAGS -DEBUG

set(__depdir "${CMAKE_CURRENT_LIST_DIR}/../opentrack-depends")

function(setq name value)
    set("${name}" "${__depdir}/${value}" CACHE INTERNAL "" FORCE)
endfunction()

set(opentrack_install-debug-info TRUE CACHE INTERNAL "" FORCE)
set(opentrack_maintainer-mode TRUE CACHE INTERNAL "" FORCE)

setq(EIGEN3_INCLUDE_DIR "eigen")
setq(OpenCV_DIR "opencv/build/install")
setq(SDL2_DIR "SDL2-win32")
setq(SDK_ARUCO_LIBPATH "aruco/build/src/aruco.lib")
setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")
setq(SDK_KINECT20 "Kinect-v2.0")
setq(SDK_LIBUSB "libusb-msvc-x86")
setq(SDK_PS3EYEDRIVER "PS3EYEDriver")
setq(SDK_REALSENSE "RSSDK-R3")
setq(SDK_RIFT_140 "ovr_sdk_win_23.0.0/LibOVR")
setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_VJOYSTICK "vjoystick")
setq(ONNXRuntime_INCLUDE_DIR "onnxruntime-1.10.0//include")
setq(ONNXRuntime_LIBRARY "onnxruntime-1.10.0/lib/onnxruntime.lib")
setq(ONNXRuntime_RUNTIME "onnxruntime-1.10.0/bin/onnxruntime.dll")
setq(SDK_TRACKHAT_SENSOR "../trackhat/trackhat-c-library-driver/build/install")

setq(Qt5_DIR "../qt-5.15-kde/lib/cmake/Qt5")
set(Qt5Core_DIR "${Qt5_DIR}Core" CACHE PATH "" FORCE)
set(Qt5Gui_DIR "${Qt5_DIR}Gui" CACHE PATH "" FORCE)

if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
    set(CMAKE_MAKE_PROGRAM "jom" CACHE STRING "" FORCE)
endif()
