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

setq(EIGEN3_INCLUDE_DIR "eigen")
setq(SDK_RIFT_140 "ovr_sdk_win_23.0.0/LibOVR")
setq(SDK_KINECT20 "Kinect-v2.0")
setq(SDK_VJOYSTICK "vjoystick")
setq(SDK_PS3EYEDRIVER "PS3EYEDriver")
setq(SDK_REALSENSE "RSSDK-R2")
setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")
setq(SDK_EYEWARE_BEAM "eyeware-beam-sdk")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
setq(Qt5_DIR "../qt-5.15-kde-amd64/lib/cmake/Qt5")
setq(OpenCV_DIR "opencv/build-amd64/install")
setq(SDK_ARUCO_LIBPATH "aruco/build-amd64/src/aruco.lib")
setq(SDK_LIBUSB "libusb-msvc-amd64")
setq(ONNXRuntime_DIR "onnxruntime-1.14.1-amd64")
setq(SDK_TRACKHAT_SENSOR "trackhat-c-library-driver/build-amd64/install")
setq(SDK_OSCPACK "oscpack/build-amd64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
setq(Qt5_DIR "../qt-5.15-kde/lib/cmake/Qt5")
setq(OpenCV_DIR "opencv/build/install")
setq(SDK_ARUCO_LIBPATH "aruco/build/src/aruco.lib")
setq(SDK_LIBUSB "libusb-msvc-x86")
setq(ONNXRuntime_DIR "onnxruntime-1.14.1")
setq(SDK_TRACKHAT_SENSOR "trackhat-c-library-driver/build/install")
setq(SDK_OSCPACK "oscpack/build")
else()
    message(FATAL_ERROR "unknown word size ${CMAKE_SIZEOF_VOID_P}b")
endif()

set(CMAKE_ASM_NASM_COMPILER nasm.exe CACHE FILEPATH "" FORCE)

set(Qt5Core_DIR "${Qt5_DIR}Core" CACHE PATH "" FORCE)
set(Qt5Gui_DIR "${Qt5_DIR}Gui" CACHE PATH "" FORCE)

if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
    set(CMAKE_MAKE_PROGRAM "jom" CACHE STRING "" FORCE)
endif()
