include_guard(GLOBAL)

include(opentrack-boilerplate)
find_package(OpenCV QUIET)

function(otr_install_opencv_libs)
    foreach(k core features2d calib3d flann imgcodecs imgproc videoio)
        otr_install_lib("opencv_${k}" "${opentrack-hier-pfx}")
    endforeach()
endfunction()

if(TARGET opencv_core)
    otr_install_opencv_libs()
endif()
