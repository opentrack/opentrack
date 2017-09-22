set(Qt5_DIR "D:/dev/qt-5.6.2/lib/cmake/Qt5")
set(__depdir "d:/dev/opentrack-depends/")
set(OpenCV_DIR "${__depdir}/opencv/build-msvc15")

set(opentrack_disable-i18n-update TRUE CACHE BOOL "" FORCE)

set(SDK_ARUCO_LIBPATH "${__depdir}/aruco/build-msvc/src/aruco.lib" CACHE FILEPATH "")
set(EIGEN3_INCLUDE_DIR "${__depdir}/eigen" CACHE PATH "")

set(SDK_FSUIPC "${__depdir}/fsuipc" CACHE PATH "")
set(SDK_HYDRA "${__depdir}/SixenseSDK" CACHE PATH "")

set(SDK_RIFT_025 "${__depdir}/LibOVR-025/build-msvc15" CACHE PATH "")
set(SDK_RIFT_042 "${__depdir}/LibOVR-042/build-msvc15" CACHE PATH "")
set(SDK_RIFT_080 "${__depdir}/LibOVR-080/build-msvc15" CACHE PATH "")
set(SDK_RIFT_140 "${__depdir}/LibOVR-140/build-msvc15/install" CACHE PATH "")

set(SDK_VALVE_STEAMVR "${__depdir}/steamvr" CACHE PATH "")
set(SDK_TOBII_EYEX "${__depdir}/Tobii-EyeX" CACHE PATH "")
set(SDK_VJOYSTICK "${__depdir}/vjoystick" CACHE PATH "")

set(SDK_REALSENSE "D:/RSSDK-R2" CACHE PATH "")
