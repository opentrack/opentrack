project(opentrack)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_INSTALL_RPATH FALSE)
SET(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

add_definitions(-DOPENTRACK_API)

if(UNIX)
    set(SDK_ENABLE_LIBEVDEV FALSE CACHE BOOL "libevdev virtual joystick protocol support (probably Linux only)")
endif()
if(WIN32)
    SET(SDK_FACEAPI_ONLY FALSE CACHE BOOL "FaceAPI only (MSVC 2005)")
endif()
if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console build")
endif()

IF(WIN32)
    SET(SDK_SM_FACEAPI_PATH "" CACHE PATH "SeeingMachines FaceAPI SDK path")
    SET(SDK_RIFT "" CACHE PATH "libOVR path")
    SET(SDK_HYDRA "" CACHE PATH "libSixense path")
    SET(SDK_VJOY "" CACHE PATH "VJoy SDK path")
ENDIF()

file(GLOB opentrack-faceapi-wrapper-c "faceapi/*.cpp")
include_directories(${CMAKE_SOURCE_DIR})

if(NOT SDK_FACEAPI_ONLY)
    find_package(OpenCV)
    find_package(Qt4 COMPONENTS QtCore QtGui QtDesigner QtXml QtNetwork REQUIRED)

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
    INCLUDE_DIRECTORIES(${QT_QTDESIGNER_INCLUDE_DIR})

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/qfunctionconfigurator)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ftnoir_posewidget)

    set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Path to Aruco static library")

    SET(SDK_OPENCV_STATIC FALSE CACHE BOOL "Whether OpenCV is statically linked")
    if (WIN32)
        SET(SDK_SM_FACEAPI_PATH "" CACHE PATH "SeeingMachines FaceAPI SDK path")
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
      if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        #set(CMAKE_RC_COMPILER_INIT windres)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
      else()
        #set(CMAKE_RC_COMPILER_INIT rc)
      endif()
      ENABLE_LANGUAGE(RC)
    endif(WIN32)

if(SDK_FSUIPC AND WIN32)
    include_directories("${SDK_FSUIPC}")
    link_directories("${SDK_FSUIPC}")
endif()

if(NOT SDK_FACEAPI_ONLY)
# main app

    file(GLOB opentrack-bin-c "facetracknoir/*.cpp" "facetracknoir/*.rc")
    file(GLOB opentrack-bin-h "facetracknoir/*.h")
    QT4_WRAP_CPP(opentrack-bin-moc ${opentrack-bin-h})
    file(GLOB opentrack-bin-ui "facetracknoir/*.ui")
    file(GLOB opentrack-bin-rc "facetracknoir/*.qrc")
    QT4_WRAP_UI(opentrack-bin-uih ${opentrack-bin-ui})
    QT4_ADD_RESOURCES(opentrack-bin-rcc ${opentrack-bin-rc})

    file(GLOB opentrack-pose-widget-c "ftnoir_posewidget/*.cpp")
    file(GLOB opentrack-pose-widget-h "ftnoir_posewidget/*.h")
    QT4_WRAP_CPP(opentrack-pose-widget-moc ${opentrack-pose-widget-h})
    file(GLOB opentrack-pose-widget-rc "ftnoir_posewidget/*.qrc")
    QT4_ADD_RESOURCES(opentrack-pose-widget-rcc ${opentrack-pose-widget-rc})

    file(GLOB opentrack-spline-widget-c "qfunctionconfigurator/*.cpp")
    file(GLOB opentrack-spline-widget-h "qfunctionconfigurator/*.h")
    QT4_WRAP_CPP(opentrack-spline-widget-moc ${opentrack-spline-widget-h})

# filters

    file(GLOB opentrack-filter-accela-c "ftnoir_filter_accela/*.cpp")
    file(GLOB opentrack-filter-accela-h "ftnoir_filter_accela/*.h")
    QT4_WRAP_CPP(opentrack-filter-accela-moc ${opentrack-filter-accela-h})
    file(GLOB opentrack-filter-accela-ui "ftnoir_filter_accela/*.ui")
    file(GLOB opentrack-filter-accela-rc "ftnoir_filter_accela/*.qrc")
    QT4_WRAP_UI(opentrack-filter-accela-uih ${opentrack-filter-accela-ui})
    QT4_ADD_RESOURCES(opentrack-filter-accela-rcc ${opentrack-filter-accela-rc})

    file(GLOB opentrack-filter-ewma-c "ftnoir_filter_ewma2/*.cpp")
    file(GLOB opentrack-filter-ewma-h "ftnoir_filter_ewma2/*.h")
    QT4_WRAP_CPP(opentrack-filter-ewma-moc ${opentrack-filter-ewma-h})
    file(GLOB opentrack-filter-ewma-ui "ftnoir_filter_ewma2/*.ui")
    file(GLOB opentrack-filter-ewma-rc "ftnoir_filter_ewma2/*.qrc")
    QT4_WRAP_UI(opentrack-filter-ewma-uih ${opentrack-filter-ewma-ui})
    QT4_ADD_RESOURCES(opentrack-filter-ewma-rcc ${opentrack-filter-ewma-rc})

# protocols

    file(GLOB opentrack-proto-fgfs-c "ftnoir_protocol_fg/*.cpp")
    file(GLOB opentrack-proto-fgfs-h "ftnoir_protocol_fg/*.h")
    QT4_WRAP_CPP(opentrack-proto-fgfs-moc ${opentrack-proto-fgfs-h})
    file(GLOB opentrack-proto-fgfs-ui "ftnoir_protocol_fg/*.ui")
    file(GLOB opentrack-proto-fgfs-rc "ftnoir_protocol_fg/*.qrc")
    QT4_WRAP_UI(opentrack-proto-fgfs-uih ${opentrack-proto-fgfs-ui})
    QT4_ADD_RESOURCES(opentrack-proto-fgfs-rcc ${opentrack-proto-fgfs-rc})

    file(GLOB opentrack-proto-fsuipc-c "ftnoir_protocol_fsuipc/*.cpp")
    file(GLOB opentrack-proto-fsuipc-h "ftnoir_protocol_fsuipc/*.h")
    QT4_WRAP_CPP(opentrack-proto-fsuipc-moc ${opentrack-proto-fsuipc-h})
    file(GLOB opentrack-proto-fsuipc-ui "ftnoir_protocol_fsuipc/*.ui")
    file(GLOB opentrack-proto-fsuipc-rc "ftnoir_protocol_fsuipc/*.qrc")
    QT4_WRAP_UI(opentrack-proto-fsuipc-uih ${opentrack-proto-fsuipc-ui})
    QT4_ADD_RESOURCES(opentrack-proto-fsuipc-rcc ${opentrack-proto-fsuipc-rc})

    file(GLOB opentrack-proto-freetrack-c "ftnoir_protocol_ft/*.cpp")
    file(GLOB opentrack-proto-freetrack-h "ftnoir_protocol_ft/*.h")
    QT4_WRAP_CPP(opentrack-proto-freetrack-moc ${opentrack-proto-freetrack-h})
    file(GLOB opentrack-proto-freetrack-ui "ftnoir_protocol_ft/*.ui")
    file(GLOB opentrack-proto-freetrack-rc "ftnoir_protocol_ft/*.qrc")
    QT4_WRAP_UI(opentrack-proto-freetrack-uih ${opentrack-proto-freetrack-ui})
    QT4_ADD_RESOURCES(opentrack-proto-freetrack-rcc ${opentrack-proto-freetrack-rc})

    file(GLOB opentrack-proto-udp-c "ftnoir_protocol_ftn/*.cpp")
    file(GLOB opentrack-proto-udp-h "ftnoir_protocol_ftn/*.h")
    QT4_WRAP_CPP(opentrack-proto-udp-moc ${opentrack-proto-udp-h})
    file(GLOB opentrack-proto-udp-ui "ftnoir_protocol_ftn/*.ui")
    file(GLOB opentrack-proto-udp-rc "ftnoir_protocol_ftn/*.qrc")
    QT4_WRAP_UI(opentrack-proto-udp-uih ${opentrack-proto-udp-ui})
    QT4_ADD_RESOURCES(opentrack-proto-udp-rcc ${opentrack-proto-udp-rc})

    file(GLOB opentrack-proto-wine-c "ftnoir_protocol_wine/*.cpp")
    file(GLOB opentrack-proto-wine-h "ftnoir_protocol_wine/*.h")
    QT4_WRAP_CPP(opentrack-proto-wine-moc ${opentrack-proto-wine-h})
    file(GLOB opentrack-proto-wine-ui "ftnoir_protocol_wine/*.ui")
    file(GLOB opentrack-proto-wine-rc "ftnoir_protocol_wine/*.qrc")
    QT4_WRAP_UI(opentrack-proto-wine-uih ${opentrack-proto-wine-ui})
    QT4_ADD_RESOURCES(opentrack-proto-wine-rcc ${opentrack-proto-wine-rc})

    file(GLOB opentrack-proto-win32-mouse-c "ftnoir_protocol_mouse/*.cpp")
    file(GLOB opentrack-proto-win32-mouse-h "ftnoir_protocol_mouse/*.h")
    QT4_WRAP_CPP(opentrack-proto-win32-mouse-moc ${opentrack-proto-win32-mouse-h})
    file(GLOB opentrack-proto-win32-mouse-ui "ftnoir_protocol_mouse/*.ui")
    file(GLOB opentrack-proto-win32-mouse-rc "ftnoir_protocol_mouse/*.qrc")
    QT4_WRAP_UI(opentrack-proto-win32-mouse-uih ${opentrack-proto-win32-mouse-ui})
    QT4_ADD_RESOURCES(opentrack-proto-win32-mouse-rcc ${opentrack-proto-win32-mouse-rc})

    file(GLOB opentrack-proto-simconnect-c "ftnoir_protocol_sc/*.cpp" "ftnoir_protocol_sc/ftnoir-protocol-sc.rc")
    file(GLOB opentrack-proto-simconnect-h "ftnoir_protocol_sc/*.h")
    QT4_WRAP_CPP(opentrack-proto-simconnect-moc ${opentrack-proto-simconnect-h})
    file(GLOB opentrack-proto-simconnect-ui "ftnoir_protocol_sc/*.ui")
    file(GLOB opentrack-proto-simconnect-rc "ftnoir_protocol_sc/*.qrc")
    QT4_WRAP_UI(opentrack-proto-simconnect-uih ${opentrack-proto-simconnect-ui})
    QT4_ADD_RESOURCES(opentrack-proto-simconnect-rcc ${opentrack-proto-simconnect-rc})

    file(GLOB opentrack-proto-vjoy-c "ftnoir_protocol_vjoy/*.cpp")
    file(GLOB opentrack-proto-vjoy-h "ftnoir_protocol_vjoy/*.h")
    QT4_WRAP_CPP(opentrack-proto-vjoy-moc ${opentrack-proto-vjoy-h})
    file(GLOB opentrack-proto-vjoy-ui "ftnoir_protocol_vjoy/*.ui")
    file(GLOB opentrack-proto-vjoy-rc "ftnoir_protocol_vjoy/*.qrc")
    QT4_WRAP_UI(opentrack-proto-vjoy-uih ${opentrack-proto-vjoy-ui})
    QT4_ADD_RESOURCES(opentrack-proto-vjoy-rcc ${opentrack-proto-vjoy-rc})

    file(GLOB opentrack-proto-libevdev-c "ftnoir_protocol_libevdev/*.cpp")
    file(GLOB opentrack-proto-libevdev-h "ftnoir_protocol_libevdev/*.h")
    QT4_WRAP_CPP(opentrack-proto-libevdev-moc ${opentrack-proto-libevdev-h})
    file(GLOB opentrack-proto-libevdev-ui "ftnoir_protocol_libevdev/*.ui")
    file(GLOB opentrack-proto-libevdev-rc "ftnoir_protocol_libevdev/*.qrc")
    QT4_WRAP_UI(opentrack-proto-libevdev-uih ${opentrack-proto-libevdev-ui})
    QT4_ADD_RESOURCES(opentrack-proto-libevdev-rcc ${opentrack-proto-libevdev-rc})

# trackers

    file(GLOB opentrack-tracker-ht-c "ftnoir_tracker_ht/*.cpp")
    file(GLOB opentrack-tracker-ht-h "ftnoir_tracker_ht/*.h")
    QT4_WRAP_CPP(opentrack-tracker-ht-moc ${opentrack-tracker-ht-h})
    file(GLOB opentrack-tracker-ht-ui "ftnoir_tracker_ht/*.ui")
    file(GLOB opentrack-tracker-ht-rc "ftnoir_tracker_ht/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-ht-uih ${opentrack-tracker-ht-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-ht-rcc ${opentrack-tracker-ht-rc})

    file(GLOB opentrack-tracker-aruco-c "ftnoir_tracker_aruco/*.cpp")
    file(GLOB opentrack-tracker-aruco-h "ftnoir_tracker_aruco/*.h")
    QT4_WRAP_CPP(opentrack-tracker-aruco-moc ${opentrack-tracker-aruco-h})
    file(GLOB opentrack-tracker-aruco-ui "ftnoir_tracker_aruco/*.ui")
    file(GLOB opentrack-tracker-aruco-rc "ftnoir_tracker_aruco/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-aruco-uih ${opentrack-tracker-aruco-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-aruco-rcc ${opentrack-tracker-aruco-rc})

    file(GLOB opentrack-tracker-pt-c "FTNoIR_Tracker_PT/*.cpp")
    file(GLOB opentrack-tracker-pt-h "FTNoIR_Tracker_PT/*.h")
    QT4_WRAP_CPP(opentrack-tracker-pt-moc ${opentrack-tracker-pt-h})
    file(GLOB opentrack-tracker-pt-ui "FTNoIR_Tracker_PT/*.ui")
    file(GLOB opentrack-tracker-pt-rc "FTNoIR_Tracker_PT/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-pt-uih ${opentrack-tracker-pt-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-pt-rcc ${opentrack-tracker-pt-rc})

    file(GLOB opentrack-tracker-udp-c "ftnoir_tracker_udp/*.cpp")
    file(GLOB opentrack-tracker-udp-h "ftnoir_tracker_udp/*.h")
    QT4_WRAP_CPP(opentrack-tracker-udp-moc ${opentrack-tracker-udp-h})
    file(GLOB opentrack-tracker-udp-ui "ftnoir_tracker_udp/*.ui")
    file(GLOB opentrack-tracker-udp-rc "ftnoir_tracker_udp/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-udp-uih ${opentrack-tracker-udp-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-udp-rcc ${opentrack-tracker-udp-rc})
	
    file(GLOB opentrack-tracker-rift-c "ftnoir_tracker_rift/*.cpp")
    file(GLOB opentrack-tracker-rift-h "ftnoir_tracker_rift/*.h")
    QT4_WRAP_CPP(opentrack-tracker-rift-moc ${opentrack-tracker-rift-h})
    file(GLOB opentrack-tracker-rift-ui "ftnoir_tracker_rift/*.ui")
    file(GLOB opentrack-tracker-rift-rc "ftnoir_tracker_rift/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-rift-uih ${opentrack-tracker-rift-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-rift-rcc ${opentrack-tracker-rift-rc})

    file(GLOB opentrack-tracker-hydra-c "ftnoir_tracker_hydra/*.cpp")
    file(GLOB opentrack-tracker-hydra-h "ftnoir_tracker_hydra/*.h")
    QT4_WRAP_CPP(opentrack-tracker-hydra-moc ${opentrack-tracker-hydra-h})
    file(GLOB opentrack-tracker-hydra-ui "ftnoir_tracker_hydra/*.ui")
    file(GLOB opentrack-tracker-hydra-rc "ftnoir_tracker_hydra/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-hydra-uih ${opentrack-tracker-hydra-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-hydra-rcc ${opentrack-tracker-hydra-rc})

    file(GLOB opentrack-tracker-faceapi-c "ftnoir_tracker_sm/*.cpp")
    file(GLOB opentrack-tracker-faceapi-h "ftnoir_tracker_sm/*.h")
    QT4_WRAP_CPP(opentrack-tracker-faceapi-moc ${opentrack-tracker-faceapi-h})
    file(GLOB opentrack-tracker-faceapi-ui "ftnoir_tracker_sm/*.ui")
    file(GLOB opentrack-tracker-faceapi-rc "ftnoir_tracker_sm/*.qrc")
    QT4_WRAP_UI(opentrack-tracker-faceapi-uih ${opentrack-tracker-faceapi-ui})
    QT4_ADD_RESOURCES(opentrack-tracker-faceapi-rcc ${opentrack-tracker-faceapi-rc})

    file(GLOB opentrack-csv-c "ftnoir_csv/*.cpp")

# compat lib for POSIX/win32

    file(GLOB opentrack-compat-c "compat/*.cpp")

# x-plane plugin
    file(GLOB opentrack-xplane-plugin-c "x-plane-plugin/*.c")
	
# freetrack

    file(GLOB opentrack-freetrack-c "freetrackclient/*.cpp")

    if(SDK_XPLANE)
        # probably librt already included
        add_library(opentrack-xplane-plugin SHARED ${opentrack-xplane-plugin-c})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-xplane-plugin
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

    add_library(opentrack-compat SHARED ${opentrack-compat-c})
    if(NOT WIN32)
        target_link_libraries(opentrack-compat rt)
    endif()

    IF(CMAKE_BUILD_TYPE MATCHES DEBUG) 
        SET(MY_QT_LIBS ${QT_QTCORE_LIBRARY_DEBUG} ${QT_QTGUI_LIBRARY_DEBUG} ${QT_QTNETWORK_LIBRARY_DEBUG} ${QT_QTXML_LIBRARY_DEBUG})
    ELSE()
        SET(MY_QT_LIBS ${QT_QTCORE_LIBRARY_RELEASE} ${QT_QTGUI_LIBRARY_RELEASE} ${QT_QTNETWORK_LIBRARY_RELEASE} ${QT_QTXML_LIBRARY_RELEASE})
    ENDIF()

    add_library(opentrack-csv SHARED ${opentrack-csv-c})
    target_link_libraries(opentrack-csv ${MY_QT_LIBS})

    add_library(opentrack-pose-widget SHARED ${opentrack-pose-widget-c} ${opentrack-pose-widget-moc} ${opentrack-pose-widget-rcc})
    target_link_libraries(opentrack-pose-widget ${MY_QT_LIBS})
    add_library(opentrack-spline-widget SHARED ${opentrack-spline-widget-c} ${opentrack-spline-widget-moc})
    target_link_libraries(opentrack-spline-widget ${MY_QT_LIBS} ${QT_QTDESIGNER_LIBRARY_RELEASE})

    add_library(opentrack-filter-accela SHARED ${opentrack-filter-accela-c} ${opentrack-filter-accela-moc} ${opentrack-filter-accela-uih} ${opentrack-filter-accela-rcc})
    target_link_libraries(opentrack-filter-accela ${MY_QT_LIBS})

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-filter-accela
        PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_library(opentrack-filter-ewma SHARED ${opentrack-filter-ewma-uih} ${opentrack-filter-ewma-c} ${opentrack-filter-ewma-moc} ${opentrack-filter-ewma-rcc})
    target_link_libraries(opentrack-filter-ewma ${MY_QT_LIBS})

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-filter-ewma
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_library(opentrack-proto-fgfs SHARED ${opentrack-proto-fgfs-c} ${opentrack-proto-fgfs-moc} ${opentrack-proto-fgfs-uih} ${opentrack-proto-fgfs-rcc})
    target_link_libraries(opentrack-proto-fgfs ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(WIN32 AND SDK_VJOY)
        include_directories(${SDK_VJOY})
        add_library(opentrack-proto-vjoy SHARED ${opentrack-proto-vjoy-c} ${opentrack-proto-vjoy-moc} ${opentrack-proto-vjoy-uih} ${opentrack-proto-vjoy-rcc})
        target_link_libraries(opentrack-proto-vjoy ${MY_QT_LIBS} "${SDK_VJOY}/VJoy.dll")
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-proto-vjoy
                          PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt -Wl,--enable-stdcall-fixup")
        endif()
    endif()

    if(UNIX AND SDK_ENABLE_LIBEVDEV)
        add_library(opentrack-proto-libevdev SHARED ${opentrack-proto-libevdev-c} ${opentrack-proto-libevdev-moc} ${opentrack-proto-libevdev-uih} ${opentrack-proto-libevdev-rcc})
        target_link_libraries(opentrack-proto-libevdev ${MY_QT_LIBS} evdev)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-proto-libevdev
                          PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
        install(TARGETS opentrack-proto-libevdev DESTINATION .)
    endif()

    if(WIN32)
        if(SDK_FSUIPC)
            add_library(opentrack-proto-fsuipc SHARED ${opentrack-proto-fsuipc-c} ${opentrack-proto-fsuipc-moc} ${opentrack-proto-fsuipc-uih} ${opentrack-proto-fsuipc-rcc})
            target_link_libraries(opentrack-proto-fsuipc ${MY_QT_LIBS} "${SDK_FSUIPC}/FSUIPC_User.lib")
            if(MSVC)
                    set_target_properties(opentrack-proto-fsuipc PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libc")
            endif()
        endif()

        if(SDK_SIMCONNECT)
            add_library(opentrack-proto-simconnect SHARED ${opentrack-proto-simconnect-c} ${opentrack-proto-simconnect-moc} ${opentrack-proto-simconnect-uih} ${opentrack-proto-simconnect-rcc})
            target_link_libraries(opentrack-proto-simconnect ${MY_QT_LIBS})
            include_directories("${SDK_SIMCONNECT}/inc")
            target_link_libraries(opentrack-proto-simconnect "${SDK_SIMCONNECT}/lib/SimConnect.lib")
        endif()

        add_library(opentrack-proto-freetrack SHARED ${opentrack-proto-freetrack-c} ${opentrack-proto-freetrack-moc} ${opentrack-proto-freetrack-uih} ${opentrack-proto-freetrack-rcc})
        target_link_libraries(opentrack-proto-freetrack opentrack-csv ${MY_QT_LIBS} opentrack-compat)

        add_library(opentrack-proto-win32-mouse SHARED ${opentrack-proto-win32-mouse-c} ${opentrack-proto-win32-mouse-moc} ${opentrack-proto-win32-mouse-uih} ${opentrack-proto-win32-mouse-rcc})
        target_link_libraries(opentrack-proto-win32-mouse ${MY_QT_LIBS})
        if(CMAKE_COMPILER_IS_GNUCC)
            add_library(freetrackclient SHARED ${opentrack-freetrack-c} "${CMAKE_SOURCE_DIR}/freetrackclient/freetrackclient.def")
            set_target_properties(freetrackclient PROPERTIES LINK_FLAGS "-Wl,--enable-stdcall-fixup")
        else()
            add_library(freetrackclient SHARED ${opentrack-freetrack-c})
        endif()
    endif()

    add_library(opentrack-proto-udp SHARED ${opentrack-proto-udp-c} ${opentrack-proto-udp-moc} ${opentrack-proto-udp-uih} ${opentrack-proto-udp-rcc})
    target_link_libraries(opentrack-proto-udp ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-proto-udp
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(NOT WIN32 AND SDK_WINE_PREFIX)
        add_library(opentrack-proto-wine SHARED ${opentrack-proto-wine-c} ${opentrack-proto-wine-moc} ${opentrack-proto-wine-uih} ${opentrack-proto-wine-rcc})
        target_link_libraries(opentrack-proto-wine ${MY_QT_LIBS} opentrack-compat opentrack-csv)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-proto-wine
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
        if(NOT SDK_WINE_NO_WRAPPER)
            add_custom_command(
              OUTPUT opentrack-wrapper-wine.exe.so
              DEPENDS "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-main.cxx"
                      "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-posix.cxx"
                      "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-windows.cxx"
              COMMAND "${SDK_WINE_PREFIX}/bin/wineg++" -g -O2 -m32 -o
                      opentrack-wrapper-wine.exe -I "${CMAKE_SOURCE_DIR}"
                      "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-main.cxx"
                      "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-posix.cxx"
                      "${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/opentrack-wrapper-wine-windows.cxx"
                      -lrt)
            add_custom_target(wine-wrapper ALL DEPENDS opentrack-wrapper-wine.exe.so)
            add_dependencies(wine-wrapper opentrack-compat opentrack-proto-wine)
        endif()
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-proto-fgfs
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    add_library(opentrack-tracker-ht SHARED ${opentrack-tracker-ht-c} ${opentrack-tracker-ht-moc} ${opentrack-tracker-ht-uih} ${opentrack-tracker-ht-rcc})
    target_link_libraries(opentrack-tracker-ht ${MY_QT_LIBS} opentrack-compat)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-tracker-ht
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()

    if(SDK_ARUCO_LIBPATH)
        include_directories(${CMAKE_SOURCE_DIR}/ftnoir_tracker_aruco/include)
        add_library(opentrack-tracker-aruco SHARED ${opentrack-tracker-aruco-c} ${opentrack-tracker-aruco-moc} ${opentrack-tracker-aruco-uih} ${opentrack-tracker-aruco-rcc})
        target_link_libraries(opentrack-tracker-aruco ${MY_QT_LIBS} ${SDK_ARUCO_LIBPATH} ${OpenCV_LIBS})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-tracker-aruco
                          PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()
    endif()

    if(WIN32)
        target_link_libraries(opentrack-tracker-ht
                      "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
                      "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib"
                      uuid)
    endif()

    if(OpenCV_FOUND)
        include_directories(${OpenCV_INCLUDE_DIRS})
        include_directories(${OpenCV_DIR}/include)
        include_directories(${OpenCV_CONFIG_PATH}/include)
    endif()

    if(OpenCV_FOUND)
        add_library(opentrack-tracker-pt SHARED ${opentrack-tracker-pt-c} ${opentrack-tracker-pt-moc} ${opentrack-tracker-pt-uih} ${opentrack-tracker-pt-rcc})
        target_link_libraries(opentrack-tracker-pt ${MY_QT_LIBS} ${OpenCV_LIBS})
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-tracker-pt
                PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        endif()

        if(WIN32)
            target_link_libraries(opentrack-tracker-pt
                          "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
                          "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib"
                          uuid)
        endif()
    endif()

    add_library(opentrack-tracker-udp SHARED ${opentrack-tracker-udp-c} ${opentrack-tracker-udp-moc} ${opentrack-tracker-udp-uih} ${opentrack-tracker-udp-rcc})
    target_link_libraries(opentrack-tracker-udp ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
        SET_TARGET_PROPERTIES(opentrack-tracker-udp
                  PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
    endif()
	
    if(SDK_RIFT)
        include_directories("${SDK_RIFT}/Include")
        include_directories("${SDK_RIFT}/Src")
        add_library(opentrack-tracker-rift SHARED ${opentrack-tracker-rift-c} ${opentrack-tracker-rift-moc} ${opentrack-tracker-rift-uih} ${opentrack-tracker-rift-rcc})
        target_link_libraries(opentrack-tracker-rift ${MY_QT_LIBS})
                if(MSVC)
                        target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/Lib/Win32/libovr.lib" winmm.lib)
                else(WIN32)
                    target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/LibOVR.a" winmm.lib setupapi.lib)
                else()
                    target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/LibOVR.a")
		endif()
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-tracker-rift
                      PROPERTIES
                      LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt"
                      COMPILE_FLAGS "-fno-strict-aliasing"
            )
        endif()
	endif()

    if(SDK_HYDRA)
        include_directories("${SDK_HYDRA}/include")
        include_directories("${SDK_HYDRA}/include/sixense_utils")
        add_library(opentrack-tracker-hydra SHARED ${opentrack-tracker-hydra-c} ${opentrack-tracker-hydra-moc} ${opentrack-tracker-hydra-uih} ${opentrack-tracker-hydra-rcc})
        target_link_libraries(opentrack-tracker-hydra ${MY_QT_LIBS})
        if(WIN32)
                if(CMAKE_COMPILER_IS_GNUCXX)
                    target_link_libraries(opentrack-tracker-hydra "${SDK_HYDRA}/lib/win32/release_dll/sixense.lib" "${SDK_HYDRA}/lib/win32/release_dll/sixense_utils.lib")
                    install(FILES "${SDK_HYDRA}/bin/win32/release_dll/sixense.dll" "${SDK_HYDRA}/bin/win32/release_dll/sixense_utils.dll"
                            DESTINATION .)
                else()
                    target_link_libraries(opentrack-tracker-hydra "${SDK_HYDRA}/lib/win32/release_static/sixense_s.lib" "${SDK_HYDRA}/lib/win32/release_static/sixense_utils_s.lib")
                endif()
        endif()
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
            SET_TARGET_PROPERTIES(opentrack-tracker-hydra
                      PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/posix-version-script.txt")
        else()
            if(MSVC)
                SET_TARGET_PROPERTIES(opentrack-tracker-hydra PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
            endif()
        endif()
    endif()
    
    if(WIN32 AND NOT SDK_CONSOLE_DEBUG)
        set(opentrack-win32-executable WIN32)
    else()
        set(opentrack-win32-executable "")
    endif()
    add_executable(opentrack ${opentrack-win32-executable} ${opentrack-bin-c} ${opentrack-bin-moc} ${opentrack-bin-uih} ${opentrack-bin-rcc})
    if(WIN32)
        target_link_libraries(opentrack "${CMAKE_SOURCE_DIR}/dinput/dinput8.lib"
                                        "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
                                        "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib"
                                        winmm
                                        uuid
        )
    endif()
    if(MSVC)
        SET_TARGET_PROPERTIES(opentrack PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    endif()
    target_link_libraries(opentrack opentrack-pose-widget opentrack-spline-widget ${MY_QT_LIBS} ${QXT_QXTCORE_LIB_RELEASE} ${QXT_QXTGUI_LIB_RELEASE})
    if(NOT WIN32)
        target_link_libraries(opentrack dl)
    endif()
    set_target_properties(opentrack PROPERTIES COMPILE_FLAGS -DOPENTRACK_MAIN)
endif()

# make install

if(SDK_SM_FACEAPI_PATH)
    include_directories("${SDK_SM_FACEAPI_PATH}/include")
    if(WIN32 AND NOT SDK_FACEAPI_ONLY)
        add_library(opentrack-tracker-faceapi SHARED ${opentrack-tracker-faceapi-c} ${opentrack-tracker-faceapi-moc} ${opentrack-tracker-faceapi-uih} ${opentrack-tracker-faceapi-rcc})
        target_link_libraries(opentrack-tracker-faceapi ${MY_QT_LIBS} opentrack-compat)
    endif()
    if(MSVC80 OR MINGW)
        add_executable(opentrack-faceapi-wrapper ${opentrack-faceapi-wrapper-c})
        target_link_libraries(opentrack-faceapi-wrapper "${SDK_SM_FACEAPI_PATH}/bin/smft32.lib")
    endif()
endif() 

install(FILES "${CMAKE_SOURCE_DIR}/README.md" DESTINATION .)

if(NOT SDK_FACEAPI_ONLY)
    if(SDK_XPLANE)
        install(TARGETS opentrack-xplane-plugin LIBRARY DESTINATION .  NAMELINK_SKIP)
    endif()
    if(WIN32)
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tracker-ht" DESTINATION .)
		install(TARGETS freetrackclient RUNTIME DESTINATION .)
    endif()

    install(FILES "${CMAKE_SOURCE_DIR}/bin/NPClient.dll" "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll" "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe" DESTINATION .)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/settings" "${CMAKE_SOURCE_DIR}/facetracknoir/clientfiles" DESTINATION .)

    if(NOT WIN32 AND SDK_WINE_PREFIX)
        install(FILES "${CMAKE_BINARY_DIR}/opentrack-wrapper-wine.exe.so"
                DESTINATION .)
        install(TARGETS opentrack-proto-wine DESTINATION .)
    endif()

    if(OpenCV_FOUND)
        install(TARGETS opentrack-tracker-pt RUNTIME DESTINATION . LIBRARY DESTINATION .)
    endif()

    if(SDK_ARUCO_LIBPATH)
        install(TARGETS opentrack-tracker-aruco RUNTIME DESTINATION . LIBRARY DESTINATION .)
    endif()

    install(TARGETS
            opentrack-compat
            opentrack-csv
            opentrack-pose-widget
            opentrack-spline-widget
            opentrack-filter-accela
            opentrack-filter-ewma
            opentrack-proto-fgfs
            opentrack-proto-udp
            opentrack-tracker-ht
            opentrack-tracker-udp
            opentrack
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
    )

    if(WIN32 AND SDK_VJOY)
        install(TARGETS opentrack-proto-vjoy
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP)
        install(FILES "${SDK_VJOY}/VJoy.dll" DESTINATION .)
    endif()

    if(SDK_RIFT)
        install(
            TARGETS opentrack-tracker-rift
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
        )
    endif()

    if(SDK_HYDRA)
        install(
            TARGETS opentrack-tracker-hydra
            RUNTIME DESTINATION .
            LIBRARY DESTINATION .
            NAMELINK_SKIP
        )
    endif()

    if(WIN32)
        install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
        if(SDK_SIMCONNECT)
        install(TARGETS opentrack-proto-simconnect
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP)
        endif()
        if(OpenCV_FOUND AND NOT SDK_OPENCV_STATIC)
            get_filename_component(opentrack-opencv-dir "${OpenCV_LIB_DIR_OPT}" PATH)
            if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
                set(LIB-PREFIX "../bin/Release/")
            else()
                set(LIB-PREFIX "lib")
            endif()
                file(GLOB opentrack-opencv-files
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_calib3d*.dll"
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_core*.dll"
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_highgui*.dll"
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_imgproc*.dll"
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_flann*.dll"
                    "${opentrack-opencv-dir}/${LIB-PREFIX}opencv_features2d*.dll"
                )
                install(FILES ${opentrack-opencv-files} DESTINATION .)
        endif()
        if(WIN32 AND NOT CMAKE_COMPILER_IS_GNUCC)
            GET_FILENAME_COMPONENT(qt-dirname "${QT_QTCORE_LIBRARY_RELEASE}" PATH)
            SET(qt-dirname "${qt-dirname}/../bin/")
            install(FILES
                "${qt-dirname}/QtCore4.dll"
                "${qt-dirname}/QtGui4.dll"
                "${qt-dirname}/QtNetwork4.dll"
                "${qt-dirname}/QtSvg4.dll"
                "${qt-dirname}/QtXml4.dll"
                DESTINATION .
            )
        endif()
        install(TARGETS
                opentrack-proto-freetrack
                opentrack-proto-win32-mouse
                #opentrack-proto-simconnect
                RUNTIME DESTINATION .
                LIBRARY DESTINATION .
                NAMELINK_SKIP
        )
        if(SDK_FSUIPC)
            install(TARGETS opentrack-proto-fsuipc
                    RUNTIME DESTINATION .
                    LIBRARY DESTINATION .
                    NAMELINK_SKIP)
        endif()

    endif()
endif()
if(SDK_SM_FACEAPI_PATH AND NOT SDK_FACEAPI_ONLY)
    install(TARGETS opentrack-tracker-faceapi
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
        NAMELINK_SKIP
    )
endif()

if(MSVC)
    file(GLOB pdbs "${CMAKE_BINARY_DIR}/*.pdb" "${CMAKE_BINARY_DIR}/release/*.pdb")
    install(FILES ${pdbs} DESTINATION .)
endif()

if(SDK_SM_FACEAPI_PATH AND (MSVC80 OR MINGW))
    install(TARGETS opentrack-faceapi-wrapper
        RUNTIME DESTINATION faceapi
        LIBRARY DESTINATION faceapi
        NAMELINK_SKIP
    )
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
