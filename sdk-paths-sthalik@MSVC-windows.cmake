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

set(opentrack-use-onnxruntime-avx-dispatch 1)
if(CMAKE_SIZEOF_VOID_P GREATER 4)
    setq(Qt6_DIR "../qt-6.10.1-msvc/lib/cmake/Qt6"
         OpenCV_DIR "opencv/build/msvc/install"
         SDK_ARUCO_LIBPATH "aruco/build/msvc/src/aruco.lib"
         SDK_LIBUSB "libusb-msvc"
         SDK_GAMEINPUT "gameinput"
         SDK_OSCPACK "oscpack/build/msvc"
         ONNXRuntime_DIR "onnxruntime-1.23.2-msvc-noavx")
    install(FILES "${__depdir}/onnxruntime-1.23.2-msvc-noavx/bin/onnxruntime.dll" RENAME "onnxruntime-noavx.dll" DESTINATION "modules")
    install(FILES "${__depdir}/onnxruntime-1.23.2-msvc-avx/bin/onnxruntime.dll" RENAME "onnxruntime-avx.dll" DESTINATION "modules")
else()
    setq(Qt6_DIR "../qt-6.9.1-msvc-x86/lib/cmake/Qt6"
         OpenCV_DIR "opencv/build/msvc-x86/install"
         SDK_ARUCO_LIBPATH "aruco/build/msvc-x86/src/aruco.lib"
         SDK_LIBUSB "libusb-msvc-x86"
         ONNXRuntime_DIR "onnxruntime-1.23.2-msvc-msvc-avx-x86"
         SDK_OSCPACK "oscpack/build/msvc-x86")
endif()

if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
    set(CMAKE_MAKE_PROGRAM "jom" CACHE STRING "" FORCE)
endif()
