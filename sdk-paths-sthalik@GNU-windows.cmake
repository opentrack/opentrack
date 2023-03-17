#
# qtbase configure line for safekeeping
#

# "../configure" -prefix d:\dev\qt-5.10.0 -no-ico -no-gif -no-libjpeg   \
# -opengl desktop -no-angle -no-fontconfig -no-harfbuzz                 \
# -nomake tests -no-mp -release -opensource -shared -confirm-license    \
# -no-freetype -force-debug-fo -make-tool make.exe -platform win32-g++

set(__depdir "${CMAKE_CURRENT_LIST_DIR}/../opentrack-depends")

function(setq name value)
    set("${name}" "${__depdir}/${value}" CACHE PATH "" FORCE)
endfunction()

set(opentrack_install-debug-info TRUE CACHE INTERNAL "" FORCE)

#setq(OpenCV_DIR "opencv/build-mingw-64")
setq(SDK_ARUCO_LIBPATH "aruco/build-mingw-w64/src/libaruco.a")

setq(EIGEN3_INCLUDE_DIR "eigen")

setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")

setq(SDK_RIFT_140 "LibOVR-140/build-mingw-w64")

setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_TOBII_EYEX "Tobii-EyeX")
setq(SDK_VJOYSTICK "vjoystick")

setq(SDK_REALSENSE "RSSDK-R2")

setq(SDK_EYEWARE_BEAM "eyeware-beam-sdk")

set(base-flags "-Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS "-std=c11 ${base-flags} -ggdb ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++17 ${base-flags} -ggdb ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math ${CMAKE_C_FLAGS_RELEASE}")

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "-ggdb ${CMAKE_C_FLAGS_DEBUG}")
