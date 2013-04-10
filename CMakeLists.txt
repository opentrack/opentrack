project(facetracknoir)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX})
#SET(CMAKE_SKIP_RPATH FALSE)

SET(SDK_FACEAPI_ONLY FALSE CACHE BOOL "FaceAPI only (MSVC 2005)")

IF(WIN32)
    SET(SDK_SM_FACEAPI_PATH "" CACHE PATH "SeeingMachines FaceAPI SDK path")
ENDIF()

file(GLOB ftnoir-faceapi-wrapper-c "faceapi/*.cpp")
include_directories(${CMAKE_SOURCE_DIR})

if(NOT SDK_FACEAPI_ONLY)
    find_package(OpenCV REQUIRED)
    find_package(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL QtNetwork REQUIRED)
    macro(QT4_WRAP_CPP outfiles)
      # get include dirs
      QT4_GET_MOC_FLAGS(moc_flags)
      QT4_EXTRACT_OPTIONS(moc_files moc_options ${ARGN})

      foreach(it ${moc_files})
        get_filename_component(it ${it} ABSOLUTE)
        QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
        SET(moc_options ${moc_options} -nn)
        QT4_CREATE_MOC_COMMAND(${it} ${outfile} "${moc_flags}" "${moc_options}")
        set(${outfiles} ${${outfiles}} ${outfile})
      endforeach()
    endmacro()

    find_package(OpenGL REQUIRED)

    if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
        add_definitions(/D FTNOIR_FILTER_BASE_LIB /D FTNOIR_TRACKER_BASE_LIB /D FTNOIR_PROTOCOL_BASE_LIB)
    else(MSVC)
        add_definitions(-DFTNOIR_FILTER_BASE_LIB -DFTNOIR_TRACKER_BASE_LIB -DFTNOIR_PROTOCOL_BASE_LIB)
    endif(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)

    if(NOT WIN32)
        if(NOT QXT_QXTGUI_LIB_RELEASE)
            find_package(Qxt COMPONENTS QxtGui REQUIRED)
        endif()

        if(NOT QXT_QXTGUI_INCLUDE_DIR OR QXT_QXTGUI_INCLUDE_DIR STREQUAL "QXT_QXTGUI_INCLUDE_DIR-NOTFOUND")
            message(FATAL_ERROR "Qxt not found, required for keybindings, get from http://libqxt.org")
        endif()

        include_directories(${QXT_QXTGUI_INCLUDE_DIR})
        include_directories(${QXT_QXTCORE_INCLUDE_DIR})
        if(EXISTS "${QXT_QXTCORE_INCLUDE_DIR}/QxtCore/")
            include_directories(${QXT_QXTCORE_INCLUDE_DIR}/QxtCore)
        endif()
        if(EXISTS "${QXT_QXTGUI_INCLUDE_DIR}/QxtGui/")
            include_directories(${QXT_QXTGUI_INCLUDE_DIR}/QxtGui)
        endif()
    endif()
    if(NOT WIN32)
        INCLUDE_DIRECTORIES(${QT_QTDESIGNER_INCLUDE_DIR})
    endif()

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/qfunctionconfigurator)

    SET(SDK_OPENCV_STATIC FALSE CACHE BOOL "Whether OpenCV is statically linked")
    if (WIN32)
        SET(SDK_SM_FACEAPI_PATH "" CACHE PATH "SeeingMachines FaceAPI SDK path")
		if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
			set(SDK_MINGW_PATH "" CACHE PATH "MinGW path")
		endif()
        set(SDK_SIMCONNECT "" CACHE PATH "Path to SimConnect SDK")
        set(SDK_DIRECTX "" CACHE PATH "Path to DirectX SDK")
        set(SDK_FSUIPC "" CACHE PATH "Path to FSUIPC")
        if(SDK_DIRECTX)
            include_directories("${SDK_DIRECTX}/Include")
            link_directories("${SDK_DIRECTX}/Lib")
        endif()
    endif()

    if(NOT WIN32)
        set(SDK_WINE_PREFIX "" CACHE PATH "Path where Wine is installed")
        set(SDK_WINE_NO_WRAPPER FALSE CACHE BOOL "Don't build wrapper, for instance X-Plane is native Linux app")
    endif()
    IF("${CMAKE_SYSTEM}" MATCHES "Linux")
        set(SDK_XPLANE "" CACHE PATH "Path to X-Plane SDK")
    endif()

    if(SDK_XPLANE)
        INCLUDE_DIRECTORIES("${SDK_XPLANE}/CHeaders" "${SDK_XPLANE}/CHeaders/XPLM")
    endif()

    if(WIN32)
      ENABLE_LANGUAGE(RC)
      if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        set(CMAKE_RC_COMPILER_INIT windres)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
      else()
        set(CMAKE_RC_COMPILER_INIT rc)
      endif()
    endif(WIN32)

if(SDK_FSUIPC AND WIN32)
    include_directories("${SDK_FSUIPC}")
    link_directories("${SDK_FSUIPC}")
endif()

if(NOT SDK_FACEAPI_ONLY)
# main app

    file(GLOB ftnoir-bin-c "facetracknoir/*.cpp" "facetracknoir/*.rc")
    file(GLOB ftnoir-bin-h "facetracknoir/*.h")
    QT4_WRAP_CPP(ftnoir-bin-moc ${ftnoir-bin-h})
    file(GLOB ftnoir-bin-ui "facetracknoir/*.ui")
    file(GLOB ftnoir-bin-rc "facetracknoir/*.qrc")
    QT4_WRAP_UI(ftnoir-bin-uih ${ftnoir-bin-ui})
    QT4_ADD_RESOURCES(ftnoir-bin-rcc ${ftnoir-bin-rc})

    file(GLOB ftnoir-pose-widget-c "ftnoir_posewidget/*.cpp")
    file(GLOB ftnoir-pose-widget-h "ftnoir_posewidget/*.h")
    QT4_WRAP_CPP(ftnoir-pose-widget-moc ${ftnoir-pose-widget-h})
    file(GLOB ftnoir-pose-widget-rc "ftnoir_posewidget/*.qrc")
    QT4_ADD_RESOURCES(ftnoir-pose-widget-rcc ${ftnoir-pose-widget-rc})

    file(GLOB ftnoir-spline-widget-c "qfunctionconfigurator/*.cpp")
    file(GLOB ftnoir-spline-widget-h "qfunctionconfigurator/*.h")
    QT4_WRAP_CPP(ftnoir-spline-widget-moc ${ftnoir-spline-widget-h})

# filters

    file(GLOB ftnoir-filter-accela-c "ftnoir_filter_accela/*.cpp")
    file(GLOB ftnoir-filter-accela-h "ftnoir_filter_accela/*.h")
    QT4_WRAP_CPP(ftnoir-filter-accela-moc ${ftnoir-filter-accela-h})
    file(GLOB ftnoir-filter-accela-ui "ftnoir_filter_accela/*.ui")
    file(GLOB ftnoir-filter-accela-rc "ftnoir_filter_accela/*.qrc")
    QT4_WRAP_UI(ftnoir-filter-accela-uih ${ftnoir-filter-accela-ui})
    QT4_ADD_RESOURCES(ftnoir-filter-accela-rcc ${ftnoir-filter-accela-rc})

    file(GLOB ftnoir-filter-kalman-c "ftnoir_filter_kalman/*.cpp")
    file(GLOB ftnoir-filter-kalman-h "ftnoir_filter_kalman/*.h")
    QT4_WRAP_CPP(ftnoir-filter-kalman-moc ${ftnoir-filter-kalman-h})
    file(GLOB ftnoir-filter-kalman-ui "ftnoir_filter_kalman/*.ui")
    file(GLOB ftnoir-filter-kalman-rc "ftnoir_filter_kalman/*.qrc")
    QT4_WRAP_UI(ftnoir-filter-kalman-uih ${ftnoir-filter-kalman-ui})
    QT4_ADD_RESOURCES(ftnoir-kalman-accela-rcc ${ftnoir-filter-kalman-rc})

    file(GLOB ftnoir-filter-ewma-c "ftnoir_filter_ewma2/*.cpp")
    file(GLOB ftnoir-filter-ewma-h "ftnoir_filter_ewma2/*.h")
    QT4_WRAP_CPP(ftnoir-filter-ewma-moc ${ftnoir-filter-ewma-h})
    file(GLOB ftnoir-filter-ewma-ui "ftnoir_filter_ewma2/*.ui")
    file(GLOB ftnoir-filter-ewma-rc "ftnoir_filter_ewma2/*.qrc")
    QT4_WRAP_UI(ftnoir-filter-ewma-uih ${ftnoir-filter-ewma-ui})
    QT4_ADD_RESOURCES(ftnoir-filter-ewma-rcc ${ftnoir-filter-ewma-rc})

# protocols

    file(GLOB ftnoir-proto-fgfs-c "ftnoir_protocol_fg/*.cpp")
    file(GLOB ftnoir-proto-fgfs-h "ftnoir_protocol_fg/*.h")
    QT4_WRAP_CPP(ftnoir-proto-fgfs-moc ${ftnoir-proto-fgfs-h})
    file(GLOB ftnoir-proto-fgfs-ui "ftnoir_protocol_fg/*.ui")
    file(GLOB ftnoir-proto-fgfs-rc "ftnoir_protocol_fg/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-fgfs-uih ${ftnoir-proto-fgfs-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-fgfs-rcc ${ftnoir-proto-fgfs-rc})

    file(GLOB ftnoir-proto-fsuipc-c "ftnoir_protocol_fsuipc/*.cpp")
    file(GLOB ftnoir-proto-fsuipc-h "ftnoir_protocol_fsuipc/*.h")
    QT4_WRAP_CPP(ftnoir-proto-fsuipc-moc ${ftnoir-proto-fsuipc-h})
    file(GLOB ftnoir-proto-fsuipc-ui "ftnoir_protocol_fsuipc/*.ui")
    file(GLOB ftnoir-proto-fsuipc-rc "ftnoir_protocol_fsuipc/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-fsuipc-uih ${ftnoir-proto-fsuipc-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-fsuipc-rcc ${ftnoir-proto-fsuipc-rc})

    file(GLOB ftnoir-proto-freetrack-c "ftnoir_protocol_ft/*.cpp")
    file(GLOB ftnoir-proto-freetrack-h "ftnoir_protocol_ft/*.h")
    QT4_WRAP_CPP(ftnoir-proto-freetrack-moc ${ftnoir-proto-freetrack-h})
    file(GLOB ftnoir-proto-freetrack-ui "ftnoir_protocol_ft/*.ui")
    file(GLOB ftnoir-proto-freetrack-rc "ftnoir_protocol_ft/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-freetrack-uih ${ftnoir-proto-freetrack-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-freetrack-rcc ${ftnoir-proto-freetrack-rc})

    file(GLOB ftnoir-proto-udp-c "ftnoir_protocol_ftn/*.cpp")
    file(GLOB ftnoir-proto-udp-h "ftnoir_protocol_ftn/*.h")
    QT4_WRAP_CPP(ftnoir-proto-udp-moc ${ftnoir-proto-udp-h})
    file(GLOB ftnoir-proto-udp-ui "ftnoir_protocol_ftn/*.ui")
    file(GLOB ftnoir-proto-udp-rc "ftnoir_protocol_ftn/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-udp-uih ${ftnoir-proto-udp-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-udp-rcc ${ftnoir-proto-udp-rc})

    file(GLOB ftnoir-proto-wine-c "ftnoir_protocol_wine/*.cpp")
    file(GLOB ftnoir-proto-wine-h "ftnoir_protocol_wine/*.h")
    QT4_WRAP_CPP(ftnoir-proto-wine-moc ${ftnoir-proto-wine-h})
    file(GLOB ftnoir-proto-wine-ui "ftnoir_protocol_wine/*.ui")
    file(GLOB ftnoir-proto-wine-rc "ftnoir_protocol_wine/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-wine-uih ${ftnoir-proto-wine-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-wine-rcc ${ftnoir-proto-wine-rc})

    file(GLOB ftnoir-proto-win32-mouse-c "ftnoir_protocol_mouse/*.cpp")
    file(GLOB ftnoir-proto-win32-mouse-h "ftnoir_protocol_mouse/*.h")
    QT4_WRAP_CPP(ftnoir-proto-win32-mouse-moc ${ftnoir-proto-win32-mouse-h})
    file(GLOB ftnoir-proto-win32-mouse-ui "ftnoir_protocol_mouse/*.ui")
    file(GLOB ftnoir-proto-win32-mouse-rc "ftnoir_protocol_mouse/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-win32-mouse-uih ${ftnoir-proto-win32-mouse-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-win32-mouse-rcc ${ftnoir-proto-win32-mouse-rc})

    file(GLOB ftnoir-proto-simconnect-c "ftnoir_protocol_sc/*.cpp")
    file(GLOB ftnoir-proto-simconnect-h "ftnoir_protocol_sc/*.h")
    QT4_WRAP_CPP(ftnoir-proto-simconnect-moc ${ftnoir-proto-simconnect-h})
    file(GLOB ftnoir-proto-simconnect-ui "ftnoir_protocol_sc/*.ui")
    file(GLOB ftnoir-proto-simconnect-rc "ftnoir_protocol_sc/*.qrc")
    QT4_WRAP_UI(ftnoir-proto-simconnect-uih ${ftnoir-proto-simconnect-ui})
    QT4_ADD_RESOURCES(ftnoir-proto-simconnect-rcc ${ftnoir-proto-simconnect-rc})

# trackers

    file(GLOB ftnoir-tracker-ht-c "ftnoir_tracker_ht/*.cpp")
    file(GLOB ftnoir-tracker-ht-h "ftnoir_tracker_ht/*.h")
    QT4_WRAP_CPP(ftnoir-tracker-ht-moc ${ftnoir-tracker-ht-h})
    file(GLOB ftnoir-tracker-ht-ui "ftnoir_tracker_ht/*.ui")
    file(GLOB ftnoir-tracker-ht-rc "ftnoir_tracker_ht/*.qrc")
    QT4_WRAP_UI(ftnoir-tracker-ht-uih ${ftnoir-tracker-ht-ui})
    QT4_ADD_RESOURCES(ftnoir-tracker-ht-rcc ${ftnoir-tracker-ht-rc})

    file(GLOB ftnoir-tracker-pt-c "ftnoir_tracker_pt/*.cpp")
    file(GLOB ftnoir-tracker-pt-h "ftnoir_tracker_pt/*.h")
    QT4_WRAP_CPP(ftnoir-tracker-pt-moc ${ftnoir-tracker-pt-h})
    file(GLOB ftnoir-tracker-pt-ui "ftnoir_tracker_pt/*.ui")
    file(GLOB ftnoir-tracker-pt-rc "ftnoir_tracker_pt/*.qrc")
    QT4_WRAP_UI(ftnoir-tracker-pt-uih ${ftnoir-tracker-pt-ui})
    QT4_ADD_RESOURCES(ftnoir-tracker-pt-rcc ${ftnoir-tracker-pt-rc})

    file(GLOB ftnoir-tracker-udp-c "ftnoir_tracker_udp/*.cpp")
    file(GLOB ftnoir-tracker-udp-h "ftnoir_tracker_udp/*.h")
    QT4_WRAP_CPP(ftnoir-tracker-udp-moc ${ftnoir-tracker-udp-h})
    file(GLOB ftnoir-tracker-udp-ui "ftnoir_tracker_udp/*.ui")
    file(GLOB ftnoir-tracker-udp-rc "ftnoir_tracker_udp/*.qrc")
    QT4_WRAP_UI(ftnoir-tracker-udp-uih ${ftnoir-tracker-udp-ui})
    QT4_ADD_RESOURCES(ftnoir-tracker-udp-rcc ${ftnoir-tracker-udp-rc})

    file(GLOB ftnoir-tracker-faceapi-c "ftnoir_tracker_sm/*.cpp")
    file(GLOB ftnoir-tracker-faceapi-h "ftnoir_tracker_sm/*.h")
    QT4_WRAP_CPP(ftnoir-tracker-faceapi-moc ${ftnoir-tracker-faceapi-h})
    file(GLOB ftnoir-tracker-faceapi-ui "ftnoir_tracker_sm/*.ui")
    file(GLOB ftnoir-tracker-faceapi-rc "ftnoir_tracker_sm/*.qrc")
    QT4_WRAP_UI(ftnoir-tracker-faceapi-uih ${ftnoir-tracker-faceapi-ui})
    QT4_ADD_RESOURCES(ftnoir-tracker-faceapi-rcc ${ftnoir-tracker-faceapi-rc})

# compat lib for POSIX/win32

    file(GLOB ftnoir-compat-c "compat/*.cpp")

# x-plane plugin
    file(GLOB ftnoir-xplane-plugin-c "x-plane-plugin/*.c")
	
# freetrack

	file(GLOB ftnoir-freetrack-c "freetrackclient/*.cpp")

    if(SDK_XPLANE)
        # probably librt already included
        add_library(ftnoir-xplane-plugin SHARED ${ftnoir-xplane-plugin-c})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(ftnoir-xplane-plugin
                PROPERTIES LINK_FLAGS
                "-Wl,--version-script=${CMAKE_SOURCE_DIR}/x-plane-plugin/version-script.txt -shared -rdynamic -nodefaultlibs -undefined_warning -fPIC"
                COMPILE_FLAGS "-Wall -O2 -pipe -fPIC -DLIN -DXPLM210"
                LIBRARY_OUTPUT_NAME "facetracknoir.xpl"
                PREFIX "" SUFFIX "")
        endif()
    endif()
endif()

# some boilerplate

    if(QT_USE_FILE)
        INCLUDE(${QT_USE_FILE})
    endif()

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        SET (CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS_INIT} -rdynamic CACHE STRING "Flags used by the linker during the creation of dll's.")
        SET (CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS_INIT} -rdynamic CACHE STRING "Flags used by the linker during the creation of modules.")
        SET (CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS_INIT} -rdynamic CACHE STRING "Flags used by the linker during the creation of modules.")
    endif()


    add_library(ftnoir-compat SHARED ${ftnoir-compat-c})
    if(NOT WIN32)
        target_link_libraries(ftnoir-compat rt)
    endif()


    IF(CMAKE_BUILD_TYPE MATCHES DEBUG) 
        SET(MY_QT_LIBS ${QT_QTCORE_LIBRARY_DEBUG} ${QT_QTGUI_LIBRARY_DEBUG} ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTXML_LIBRARY_DEBUG} ${QT_QTOPENGL_LIBRARY_DEBUG})
    ELSE()
        SET(MY_QT_LIBS ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE} ${QT_QTNETWORK_LIBRARY_RELEASE} ${QT_QTXML_LIBRARY_RELEASE} ${QT_QTOPENGL_LIBRARY_RELEASE})
    ENDIF()
    add_library(ftnoir-pose-widget SHARED ${ftnoir-pose-widget-c} ${ftnoir-pose-widget-moc} ${ftnoir-pose-widget-rcc})
    target_link_libraries(ftnoir-pose-widget ${MY_QT_LIBS} ${OPENGL_gl_LIBRARY})
    add_library(ftnoir-spline-widget SHARED ${ftnoir-spline-widget-c} ${ftnoir-spline-widget-moc} ${MY_QT_LIBS})
    target_link_libraries(ftnoir-spline-widget ${MY_QT_LIBS})

    add_library(ftnoir-filter-accela SHARED ${ftnoir-filter-accela-c} ${ftnoir-filter-accela-moc} ${ftnoir-filter-accela-uih} ${ftnoir-filter-accela-rcc})
    target_link_libraries(ftnoir-filter-accela ${MY_QT_LIBS} ftnoir-spline-widget)

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-filter-accela
        PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(OpenCV_FOUND)
        include_directories(${OPENCV_INCLUDE_DIRS})
        include_directories("${OpenCV_DIR}/include")
        include_directories("${OpenCV_CONFIG_PATH}/include")
        add_library(ftnoir-filter-kalman SHARED ${ftnoir-filter-kalman-c} ${ftnoir-filter-kalman-moc} ${ftnoir-filter-kalman-uih} ${ftnoir-filter-kalman-rcc})
        target_link_libraries(ftnoir-filter-kalman ${MY_QT_LIBS} ${OpenCV_LIBS})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(ftnoir-filter-kalman
            PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
        if(WIN32)
                SET(STATIC_EXT "lib")
        else()
                SET(STATIC_EXT "a")
        endif()
    endif()

    add_library(ftnoir-filter-ewma SHARED ${ftnoir-filter-ewma-uih} ${ftnoir-filter-ewma-c} ${ftnoir-filter-ewma-moc} ${ftnoir-filter-ewma-rcc})
    target_link_libraries(ftnoir-filter-ewma ${MY_QT_LIBS})

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-filter-ewma
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_library(ftnoir-proto-fgfs SHARED ${ftnoir-proto-fgfs-c} ${ftnoir-proto-fgfs-moc} ${ftnoir-proto-fgfs-uih} ${ftnoir-proto-fgfs-rcc})
    target_link_libraries(ftnoir-proto-fgfs ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(WIN32)
        if(SDK_FSUIPC)
            add_library(ftnoir-proto-fsuipc SHARED ${ftnoir-proto-fsuipc-c} ${ftnoir-proto-fsuipc-moc} ${ftnoir-proto-fsuipc-uih} ${ftnoir-proto-fsuipc-rcc})
            target_link_libraries(ftnoir-proto-fsuipc ${MY_QT_LIBS} "${SDK_FSUIPC}/FSUIPC_User.lib")
            if(MSVC)
                    set_target_properties(ftnoir-proto-fsuipc PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc")
            endif()
        endif()

        if(SDK_SIMCONNECT)
            add_library(ftnoir-proto-simconnect SHARED ${ftnoir-proto-simconnect-c} ${ftnoir-proto-simconnect-moc} ${ftnoir-proto-simconnect-uih} ${ftnoir-proto-simconnect-rcc})
            target_link_libraries(ftnoir-proto-simconnect ${MY_QT_LIBS})
            include_directories("${SDK_SIMCONNECT}/inc")
            target_link_libraries(ftnoir-proto-simconnect "${SDK_SIMCONNECT}/lib/simconnect.lib")
        endif()

        add_library(ftnoir-proto-freetrack SHARED ${ftnoir-proto-freetrack-c} ${ftnoir-proto-freetrack-moc} ${ftnoir-proto-freetrack-uih} ${ftnoir-proto-freetrack-rcc})
        target_link_libraries(ftnoir-proto-freetrack ${MY_QT_LIBS})

        add_library(ftnoir-proto-win32-mouse SHARED ${ftnoir-proto-win32-mouse-c} ${ftnoir-proto-win32-mouse-moc} ${ftnoir-proto-win32-mouse-uih} ${ftnoir-proto-win32-mouse-rcc})
        target_link_libraries(ftnoir-proto-win32-mouse ${MY_QT_LIBS})
		add_library(freetrackclient SHARED ${ftnoir-freetrack-c})
    endif()

    add_library(ftnoir-proto-udp SHARED ${ftnoir-proto-udp-c} ${ftnoir-proto-udp-moc} ${ftnoir-proto-udp-uih} ${ftnoir-proto-udp-rcc})
    target_link_libraries(ftnoir-proto-udp ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-proto-udp
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(NOT WIN32 AND SDK_WINE_PREFIX)
        add_library(ftnoir-proto-wine SHARED ${ftnoir-proto-wine-c} ${ftnoir-proto-wine-moc} ${ftnoir-proto-wine-uih} ${ftnoir-proto-wine-rcc})
        target_link_libraries(ftnoir-proto-wine ${MY_QT_LIBS} ftnoir-compat)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(ftnoir-proto-wine
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
        if(NOT SDK_WINE_NO_WRAPPER)
            add_custom_command( TARGET ftnoir-proto-wine
                    COMMAND
                    "${SDK_WINE_PREFIX}/bin/wineg++" -g -O2 -m32 -o
                    ftnoir-wrapper-wine.exe -I "${CMAKE_SOURCE_DIR}"
                    "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/ftnoir-wrapper-wine-main.cxx"
                    "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/ftnoir-wrapper-wine-posix.cxx"
                    "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/ftnoir-wrapper-wine-windows.cxx"
                    -lrt)
        endif()
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_library(ftnoir-tracker-ht SHARED ${ftnoir-tracker-ht-c} ${ftnoir-tracker-ht-moc} ${ftnoir-tracker-ht-uih} ${ftnoir-tracker-ht-rcc})
    target_link_libraries(ftnoir-tracker-ht ${MY_QT_LIBS} ${OPENGL_gl_LIBRARY} ftnoir-compat)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-tracker-ht
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
        target_link_libraries(ftnoir-tracker-ht
                      "${CMAKE_SOURCE_DIR}/dinput/dinput8.lib"
                      "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
                      "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib")
    endif()

    if(OpenCV_FOUND)
        add_library(ftnoir-tracker-pt SHARED ${ftnoir-tracker-pt-c} ${ftnoir-tracker-pt-moc} ${ftnoir-tracker-pt-uih} ${ftnoir-tracker-pt-rcc})
        target_link_libraries(ftnoir-tracker-pt ${MY_QT_LIBS} ${OpenCV_LIBS} ${OPENGL_gl_LIBRARY})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-tracker-pt
                PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
    endif()

    add_library(ftnoir-tracker-udp SHARED ${ftnoir-tracker-udp-c} ${ftnoir-tracker-udp-moc} ${ftnoir-tracker-udp-uih} ${ftnoir-tracker-udp-rcc})
    target_link_libraries(ftnoir-tracker-udp ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(ftnoir-tracker-udp
                  PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_executable(ftnoir ${ftnoir-bin-c} ${ftnoir-bin-moc} ${ftnoir-bin-uih} ${ftnoir-bin-rcc})
    if(WIN32)
        target_link_libraries(ftnoir ftnoir-pose-widget ftnoir-spline-widget ${MY_QT_LIBS} "${CMAKE_SOURCE_DIR}/dinput/dinput8.lib" "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib" "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib")
    endif()
    target_link_libraries(ftnoir ftnoir-pose-widget ftnoir-spline-widget ${MY_QT_LIBS} ${QXT_QXTCORE_LIB_RELEASE} ${QXT_QXTGUI_LIB_RELEASE})
    if(NOT WIN32)
        target_link_libraries(ftnoir dl)
    endif()

    if(WIN32)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(ftnoir PROPERTIES LINK_FLAGS "-Wl,--subsystem,windows")
        endif()
    endif()
endif()

# make install

if(SDK_SM_FACEAPI_PATH)
    include_directories("${SDK_SM_FACEAPI_PATH}/include")
endif() 
if(WIN32 AND NOT SDK_FACEAPI_ONLY)
    add_library(ftnoir-tracker-faceapi SHARED ${ftnoir-tracker-faceapi-c} ${ftnoir-tracker-faceapi-moc} ${ftnoir-tracker-faceapi-uih} ${ftnoir-tracker-faceapi-rcc})
    target_link_libraries(ftnoir-tracker-faceapi ${MY_QT_LIBS})
endif()
if(MSVC80 AND SDK_SM_FACEAPI_PATH)
    add_executable(ftnoir-faceapi-wrapper ${ftnoir-faceapi-wrapper-c})
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        target_link_libraries(ftnoir-faceapi-wrapper "${SDK_SM_FACEAPI_PATH}/bin/smft32.lib" "${SDK_MSVC80_PATH}/lib/msvcrt.lib")
    else()
        target_link_libraries(ftnoir-faceapi-wrapper "${SDK_SM_FACEAPI_PATH}/bin/smft32.lib")
    endif()
else()
    MESSAGE(WARNING "Cannot build FaceAPI without MSVC 2005!")
endif()

if(NOT SDK_FACEAPI_ONLY)
    if(SDK_XPLANE)
        install(TARGETS ftnoir-xplane-plugin LIBRARY DESTINATION .  NAMELINK_SKIP)
    endif()
    if(WIN32)
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tracker-ht" DESTINATION .)
		install(TARGETS freetrackclient RUNTIME DESTINATION .)
    endif()

    install(FILES "${CMAKE_SOURCE_DIR}/bin/npclient.dll" "${CMAKE_SOURCE_DIR}/bin/npclient64.dll" DESTINATION .)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/settings" DESTINATION .)

    if(NOT WIN32 AND SDK_WINE_PREFIX)
        install(FILES "${CMAKE_BINARY_DIR}/ftnoir-wrapper-wine.exe.so"
                DESTINATION .)
        install(TARGETS ftnoir-proto-wine DESTINATION .)
    endif()

    install(TARGETS
            ftnoir-compat
            ftnoir-pose-widget
            ftnoir-spline-widget
            ftnoir-filter-accela
            ftnoir-filter-ewma
            ftnoir-proto-fgfs
            ftnoir-proto-udp
            ftnoir-tracker-ht
        ftnoir-tracker-udp
            ftnoir
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
    )

    if(OpenCV_FOUND)
        install(TARGETS
                ftnoir-tracker-pt
            ftnoir-filter-kalman
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP
        )
    endif()

    if(WIN32)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            file(GLOB mingw-libgcc-s "${SDK_MINGW_PATH}/bin/libgcc_s_*.dll")
            install(FILES
                "${mingw-libgcc-s}"
                "${SDK_MINGW_PATH}/lib32/libstdc++-6.dll"
                DESTINATION .
            )
        endif()
        install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
        if(SDK_SIMCONNECT)
        install(TARGETS ftnoir-proto-simconnect
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP)
        endif()
        if(OpenCV_FOUND AND NOT SDK_OPENCV_STATIC)
            get_filename_component(ftnoir-opencv-dir "${OpenCV_LIB_DIR_OPT}" PATH)
            if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
                set(LIB-PREFIX "../bin/Release/")
            else()
                set(LIB-PREFIX "lib")
            endif()
                file(GLOB ftnoir-opencv-files
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_calib3d*.dll"
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_core*.dll"
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_highgui*.dll"
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_imgproc*.dll"
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_flann*.dll"
                    "${ftnoir-opencv-dir}/${LIB-PREFIX}opencv_features2d*.dll"
                )
                install(FILES ${ftnoir-opencv-files} DESTINATION .)
        endif()
        get_filename_component(ftnoir-qt-bin "${QT_QMAKE_EXECUTABLE}" PATH)
        if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
        GET_FILENAME_COMPONENT(qt-dirname "${QT_QTCORE_LIBRARY_RELEASE}" PATH)
        SET(qt-dirname "${qt-dirname}/../bin/")
        if(NOT EXISTS "${qt-dirname}/../bin/")
            GET_FILENAME_COMPONENT(qt-dirname "${QT_QTCORE_LIBRARY_RELEASE}" PATH)
            SET(qt-dirname "${qt-dirname}/../bin/")
        endif()
        install(FILES
            "${qt-dirname}/QtCore4.dll"
            "${qt-dirname}/QtGui4.dll"
            "${qt-dirname}/QtNetwork4.dll"
            "${qt-dirname}/QtOpenGL4.dll"
            "${qt-dirname}/QtSvg4.dll"
            "${qt-dirname}/QtXml4.dll"
            DESTINATION .
        )
        endif()
        install(TARGETS
                ftnoir-proto-freetrack
                ftnoir-proto-win32-mouse
                #ftnoir-proto-simconnect
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP
        )
        if(SDK_FSUIPC)
            install(TARGETS ftnoir-proto-fsuipc
                    RUNTIME DESTINATION .
                    LIBRARY DESTINATION .
                    NAMELINK_SKIP)
        endif()

    endif()
endif()
if(WIN32 AND NOT SDK_FACEAPI_ONLY)
    install(TARGETS ftnoir-tracker-faceapi
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
        NAMELINK_SKIP
    )
endif()

if(WIN32 AND SDK_SM_FACEAPI_PATH)
    if(MSVC80)
        install(TARGETS ftnoir-faceapi-wrapper
            RUNTIME DESTINATION faceapi
            LIBRARY DESTINATION faceapi
            NAMELINK_SKIP
        )
    endif()
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION faceapi)
    install(FILES
        "${SDK_SM_FACEAPI_PATH}/bin/computation6.0.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/foundation6.0.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/image6.0.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/libpng13.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/pgrflycapturegui.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/QtCore0.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/qt-mt334.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/QtXml4.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/smft32.dll"
        "${SDK_SM_FACEAPI_PATH}/bin/zlib1.dll"
        "${SDK_SM_FACEAPI_PATH}/doc/RELEASE NOTES.txt"
        DESTINATION faceapi
    )
    install(DIRECTORY
        "${SDK_SM_FACEAPI_PATH}/bin/cal"
        "${SDK_SM_FACEAPI_PATH}/bin/resources"
        DESTINATION faceapi
    )
endif()
