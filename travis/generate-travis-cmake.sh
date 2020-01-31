#!/bin/bash

cat << EOF > sdk-paths-$LOGNAME@Clang-Darwin.cmake
set(__depdir "${CMAKE_CURRENT_LIST_DIR}/opentrack-depends")

function(set_sdk name value)
    set("${name}" "${__depdir}/${value}" CACHE INTERNAL "" FORCE)
endfunction()

set_sdk(SDK_ARUCO_LIBPATH "aruco/build/src/libaruco.a")
#set_sdk(SDK_HYDRA "SixenseSDK")
#set_sdk(SDK_VALVE_STEAMVR "steamvr")
#set_sdk(SDK_WINE TRUE)
set_sdk(SDK_XPLANE "X-Plane-SDK")
EOF
