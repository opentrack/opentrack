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

setq(SDK_RIFT_140 "ovr_sdk_win_23.0.0/LibOVR")
setq(SDK_KINECT20 "nonfree/Kinect-v2.0")
setq(SDK_VJOYSTICK "vjoystick")
setq(SDK_PS3EYEDRIVER "PS3EYEDriver")
setq(SDK_REALSENSE "nonfree/RSSDK-R2")
setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")
setq(SDK_EYEWARE_BEAM "eyeware-beam-sdk")
setq(SDK_TOBII "nonfree/tobii-streamengine")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
setq(Qt6_DIR "../qt-6.9.2-msvc-amd64/lib/cmake/Qt6")
setq(OpenCV_DIR "opencv/build/amd64/install")
setq(SDK_ARUCO_LIBPATH "aruco/build/amd64/src/aruco.lib")
setq(SDK_LIBUSB "libusb-msvc-amd64")
setq(SDK_GAMEINPUT "gameinput")
setq(ONNXRuntime_DIR "onnxruntime-1.22.1-nolto")
#setq(SDK_TRACKHAT_SENSOR "trackhat-c-library-driver/build/amd64/install")
set(SDK_TRACKHAT_SENSOR "FALSE" CACHE INTERNAL "" FORCE)
setq(SDK_OSCPACK "oscpack/build/amd64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
setq(Qt6_DIR "../qt-6.9.2-msvc-x86/lib/cmake/Qt6")
setq(OpenCV_DIR "opencv/build/x86/install")
setq(SDK_ARUCO_LIBPATH "aruco/build/x86/src/aruco.lib")
setq(SDK_LIBUSB "libusb-msvc-x86")
setq(ONNXRuntime_DIR "onnxruntime-1.22.1-nolto-x86")
setq(ONNXRuntime_DIR "")
#setq(SDK_TRACKHAT_SENSOR "trackhat-c-library-driver/build/install")
set(SDK_TRACKHAT_SENSOR "FALSE" CACHE INTERNAL "" FORCE)
setq(SDK_OSCPACK "oscpack/build/x86")
else()
    message(FATAL_ERROR "unknown word size ${CMAKE_SIZEOF_VOID_P}")
endif()

set(CMAKE_ASM_NASM_COMPILER nasm.exe CACHE FILEPATH "" FORCE)

set(qt6Core_DIR "${qt6_DIR}Core" CACHE PATH "" FORCE)
set(qt6Gui_DIR "${qt6_DIR}Gui" CACHE PATH "" FORCE)

if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
    set(CMAKE_MAKE_PROGRAM "jom" CACHE STRING "" FORCE)
endif()
