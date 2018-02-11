# qtbase configure line for safekeeping
# "../configure" -prefix d:\dev\qt-5.10.0 -no-ico -no-gif -no-libjpeg -opengl desktop -no-angle -no-fontconfig -no-harfbuzz -nomake tests -no-mp -release -opensource -shared -confirm-license -no-freetype -force-debug-info -separate-debug-info -make-tool jom -platform win32-msvc

# remember to change -MD to -MT in mkspecs/
# also add CFLAGS -Zi and LFLAGS -DEBUG

function(setq name value)
    set("${name}" "${value}" CACHE INTERNAL "" FORCE)
endfunction()

set(Qt5_DIR "D:/dev/qt-5.10.0/lib/cmake/Qt5")
set(__depdir "d:/dev/opentrack-depends/")
set(OpenCV_DIR "${__depdir}/opencv/build")

setq(opentrack_install-debug-info TRUE)

setq(SDK_ARUCO_LIBPATH "${__depdir}/aruco/build/src/aruco.lib")
setq(EIGEN3_INCLUDE_DIR "${__depdir}/eigen")

setq(SDK_FSUIPC "${__depdir}/fsuipc")
setq(SDK_HYDRA "${__depdir}/SixenseSDK")

setq(SDK_RIFT_025 "${__depdir}/LibOVR-025/build")
setq(SDK_RIFT_042 "${__depdir}/LibOVR-042/build")
setq(SDK_RIFT_080 "${__depdir}/LibOVR-080/build")
setq(SDK_RIFT_140 "${__depdir}/LibOVR-140/build")

setq(SDK_VALVE_STEAMVR "${__depdir}/steamvr")
setq(SDK_TOBII_EYEX "${__depdir}/Tobii-EyeX")
setq(SDK_VJOYSTICK "${__depdir}/vjoystick")

setq(SDK_REALSENSE "D:/RSSDK-R2")
