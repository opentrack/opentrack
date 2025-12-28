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

setq(OpenCV_DIR "opencv/build/clang-asan/install")
#setq(SDK_ARUCO_LIBPATH "aruco/build-mingw-w64/src/libaruco.a")
setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")
setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_VJOYSTICK "vjoystick")

setq(SDK_REALSENSE "nonfree/RSSDK-R2")
setq(SDK_GAMEINPUT "gameinput")
setq(Qt6_DIR "../qt-6.10.1-clang-asan-amd64/lib/cmake/Qt6")

set(base-cxxflags "-Wall -Wextra -Wpedantic")

set(CMAKE_C_FLAGS "${base-cxxflags} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${base-cxxflags} ${CMAKE_CXX_FLAGS}")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -fsanitize=bounds,address ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -fsanitize=bounds,address ${CMAKE_C_FLAGS_DEBUG}")
