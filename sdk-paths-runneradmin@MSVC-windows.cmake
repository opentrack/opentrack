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

set(__depdir "${CMAKE_CURRENT_LIST_DIR}/opentrack-depends")

function(setq name value)
    set("${name}" "${__depdir}/${value}" CACHE INTERNAL "" FORCE)
endfunction()

set(opentrack_install-debug-info FALSE CACHE INTERNAL "" FORCE)

#setq(EIGEN3_INCLUDE_DIR "eigen")
#setq(OpenCV_DIR "../opencv")
#setq(SDK_ARUCO_LIBPATH "../aruco")
#setq(SDK_FSUIPC "fsuipc")
#setq(SDK_HYDRA "SixenseSDK")
#setq(SDK_KINECT20 "../Kinect-v2.0")
#setq(SDK_LIBUSB "libusb-msvc-x86")
#setq(SDK_REALSENSE "../RSSDK-R2")
#setq(SDK_RIFT_140 "ovr_sdk_win_23.0.0/LibOVR")
#setq(SDK_VALVE_STEAMVR "steamvr")
#setq(SDK_VJOYSTICK "vjoystick")
#setq(ONNXRuntime_DIR "../onnxruntime-1.12.1")
#setq(SDK_TRACKHAT_SENSOR "../trackhat-c-library-driver")
