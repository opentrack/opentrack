project(opentrack)
cmake_minimum_required(VERSION 2.8)

include(CMakeParseArguments)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK__COMMIT --tags --always)
endif()

include_directories(${CMAKE_SOURCE_DIR})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    if(MINGW)
        set(version-script mingw)
    else()
        set(version-script posix)
    endif()
endif()

if(APPLE)
    set(apple-frameworks "-stdlib=libc++ -framework Cocoa -framework CoreFoundation -lobjc -lz -framework Carbon")
    set(CMAKE_SHARED_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS " -stdlib=libc++ -std=c++11 ${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_INSTALL_RPATH FALSE)
SET(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_definitions(-DOPENTRACK_API -DIN_OPENTRACK)

if(MSVC)
    add_definitions(-DNOMINMAX)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    add_definitions(-std=c++11)
endif()

if(WIN32 AND MSVC)
	set(SDK_GOOGLE_BREAKPAD "" CACHE PATH "google-breakpad for crash reporting")
endif()

if(MINGW)
    set(SDK_MINGW_PREFIX "" CACHE PATH "mingw prefix")
endif()

if(SDK_GOOGLE_BREAKPAD AND WIN32)
	add_definitions(-DOPENTRACK_BREAKPAD)
	include_directories("${SDK_GOOGLE_BREAKPAD}/src/client/windows/handler")
	include_directories("${SDK_GOOGLE_BREAKPAD}/src/")
endif()

if(WIN32 AND DEFINED MSVC_VERSION AND NOT ${MSVC_VERSION} LESS 1700)
		find_path (WIN8_SDK_ROOT_DIR
				Include/um/windows.h
				PATHS
						"$ENV{ProgramFiles}/Windows Kits/8.0"
						"$ENV{ProgramFiles(x86)}/Windows Kits/8.0"
						DOC "Windows 8 SDK root directory"
		)

                if(WIN8_SDK_ROOT_DIR)
		  SET(CMAKE_LIBRARY_PATH "${WIN8_SDK_ROOT_DIR}/Lib/win8/um/x86")
                endif()
endif()

if(WIN32 AND DEFINED MSVC_VERSION AND NOT ${MSVC_VERSION} LESS 1700)
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
	SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
endif()

if(UNIX)
    set(SDK_ENABLE_LIBEVDEV FALSE CACHE BOOL "libevdev virtual joystick protocol support (probably Linux only)")
endif()
if(WIN32)
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console build")
endif()

IF(WIN32)
    SET(SDK_VJOY "" CACHE PATH "VJoy SDK path")
ENDIF()
SET(SDK_HYDRA "" CACHE PATH "libSixense path")
SET(SDK_HYDRA_AMD64 FALSE CACHE BOOL "whether target is amd64 (else ia-32)")

SET(SDK_RIFT "" CACHE PATH "libOVR path")

include_directories(${CMAKE_SOURCE_DIR})

if(MINGW)
    # qt scripts are broken
    set(Qt5Gui_user32_LIBRARY ${SDK_MINGW_PREFIX}/mingw/lib/libuser32.a)
    set(Qt5Gui_opengl32_LIBRARY ${SDK_MINGW_PREFIX}/mingw/lib/libopengl32.a)
    set(Qt5Gui_glu32_LIBRARY ${SDK_MINGW_PREFIX}/mingw/lib/libglu32.a)
    set(Qt5Gui_gdi32_LIBRARY ${SDK_MINGW_PREFIX}/mingw/lib/libgdi32.a)
endif()

find_package(OpenCV REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Core Xml Network Widgets Gui ${maybe-serial-port} QUIET)
cmake_policy(SET CMP0020 NEW)
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Xml_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS})


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/qfunctionconfigurator)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ftnoir_posewidget)

set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Path to Aruco static library")

SET(SDK_OPENCV_STATIC FALSE CACHE BOOL "Whether OpenCV is statically linked")
if(WIN32)
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
IF("${CMAKE_SYSTEM}" MATCHES "Linux" OR APPLE)
    set(SDK_XPLANE "" CACHE PATH "Path to X-Plane SDK")
endif()

if(SDK_XPLANE)
    INCLUDE_DIRECTORIES("${SDK_XPLANE}/CHeaders" "${SDK_XPLANE}/CHeaders/XPLM")
endif()

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  ENABLE_LANGUAGE(RC)
endif(WIN32)

if(SDK_FSUIPC AND WIN32)
    include_directories("${SDK_FSUIPC}")
    link_directories("${SDK_FSUIPC}")
endif()

# Qxt bundled :: sorry for this, but gentoo ebuild is broken
if(APPLE)
    set(qxt-mini-c qxt-mini/plat/qxtglobalshortcut_mac.cpp qxt-mini/qxtglobalshortcut.cpp)
    include_directories("qxt-mini/")
else()
    if(UNIX)
        set(qxt-mini-c qxt-mini/plat/qxtglobalshortcut_x11.cpp qxt-mini/qxtglobalshortcut.cpp)
        include_directories("qxt-mini/")
    endif()
endif()

# qt being broken as usual
set(EXTRA-MOCS "${CMAKE_SOURCE_DIR}/facetracknoir/options.h")

function(link_with_dinput8 n)
	if(WIN32)
		if(MSVC)
			target_link_libraries(${n}
							  "${CMAKE_SOURCE_DIR}/dinput/dinput8.lib"
							  "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
							  "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib"
							  uuid)		
		else()
			target_link_libraries(${n} dinput8 dxguid strmiids)
		endif()
	endif()
endfunction()

macro(opentrack_module n dir)
    file(GLOB ${n}-c "${dir}/*.cpp" "${dir}/*.h" "${dir}/*.rc" "${dir}/*.hpp" ${EXTRA-MOCS})
    file(GLOB ${n}-ui "${dir}/*.ui")
    file(GLOB ${n}-rc "${dir}/*.qrc")
    QT5_WRAP_UI(${n}-uih ${${n}-ui})
    QT5_ADD_RESOURCES(${n}-rcc ${${n}-rc})
endmacro()

macro(opentrack_library n)
    cmake_parse_arguments(foolib "" "LINK;COMPILE" "" ${ARGN})
    if(NOT " ${foolib_UNPARSED_ARGUMENTS}" STREQUAL " ")
        message(FATAL_ERROR "opentrack_library bad formals")
    endif()
    add_library(${n} SHARED ${${n}-c} ${${n}-uih} ${${n}-rcc})
    target_link_libraries(${n} ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
        SET_TARGET_PROPERTIES(${n} PROPERTIES
            LINK_FLAGS "${foolib_LINK} -Wl,--version-script=${CMAKE_SOURCE_DIR}/facetracknoir/${version-script}-version-script.txt"
            COMPILE_FLAGS "${foolib_COMPILE}"
            )
    else()
        set_target_properties(${n} PROPERTIES LINK_FLAGS "${foolib_LINK}" COMPILE_FLAGS "${foolib_COMPILE}")
    endif()
    install(TARGETS ${n} RUNTIME DESTINATION . LIBRARY DESTINATION .)
endmacro()

file(GLOB opentrack-lib-c "opentrack-api/*.cpp" "facetracknoir/global-settings.cpp" "opentrack-api/*.h" "facetracknoir/global-settings.h")

opentrack_module(opentrack-bin facetracknoir)
opentrack_module(opentrack-pose-widget ftnoir_posewidget)
opentrack_module(opentrack-spline-widget qfunctionconfigurator)

# filters

opentrack_module(opentrack-filter-accela ftnoir_filter_accela)
opentrack_module(opentrack-filter-kalman ftnoir_filter_kalman)
opentrack_module(opentrack-filter-ewma ftnoir_filter_ewma2)

# protocols

opentrack_module(opentrack-proto-fgfs ftnoir_protocol_fg)
opentrack_module(opentrack-proto-fsuipc ftnoir_protocol_fsuipc)
opentrack_module(opentrack-proto-freetrack ftnoir_protocol_ft)
opentrack_module(opentrack-proto-udp ftnoir_protocol_ftn)
opentrack_module(opentrack-proto-wine ftnoir_protocol_wine)
opentrack_module(opentrack-proto-win32-mouse ftnoir_protocol_mouse)
opentrack_module(opentrack-proto-simconnect ftnoir_protocol_sc)
opentrack_module(opentrack-proto-vjoy ftnoir_protocol_vjoy)
opentrack_module(opentrack-proto-libevdev ftnoir_protocol_libevdev)

# trackers

opentrack_module(opentrack-tracker-ht ftnoir_tracker_ht)
opentrack_module(opentrack-tracker-aruco ftnoir_tracker_aruco)
opentrack_module(opentrack-tracker-pt FTNoIR_Tracker_PT)
opentrack_module(opentrack-tracker-udp ftnoir_tracker_udp)
opentrack_module(opentrack-tracker-joystick ftnoir_tracker_joystick)
opentrack_module(opentrack-tracker-rift ftnoir_tracker_rift)
opentrack_module(opentrack-tracker-hydra ftnoir_tracker_hydra)

file(GLOB opentrack-csv-c "ftnoir_csv/*.cpp" "ftnoir_csv/*.h")

# compat lib for POSIX/win32

file(GLOB opentrack-compat-c "compat/*.cpp" "compat/*.h")

# x-plane plugin
file(GLOB opentrack-xplane-plugin-c "x-plane-plugin/*.c")

# freetrack

file(GLOB opentrack-freetrack-c "freetrackclient/*.cpp")

if(SDK_XPLANE)
    # probably librt already included
    add_library(opentrack-xplane-plugin SHARED ${opentrack-xplane-plugin-c})
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
        SET_TARGET_PROPERTIES(opentrack-xplane-plugin
            PROPERTIES LINK_FLAGS
            "-Wl,--version-script=${CMAKE_SOURCE_DIR}/x-plane-plugin/version-script.txt -shared -rdynamic -nodefaultlibs -undefined_warning -fPIC"
            COMPILE_FLAGS "-Wall -O2 -pipe -fPIC -DLIN -DXPLM210"
            LIBRARY_OUTPUT_NAME "opentrack.xpl"
            PREFIX "" SUFFIX "")
    endif()
    if(APPLE)
        SET_TARGET_PROPERTIES(opentrack-xplane-plugin PROPERTIES
                              COMPILE_FLAGS "-iframework ${SDK_XPLANE}/Libraries/Mac/ -DAPL -DXPLM210 -framework XPLM -framework XPWidgets"
                              LINK_FLAGS "-F${SDK_XPLANE}/Libraries/Mac/ -framework XPLM -framework XPWidgets")
    endif()
    if(UNIX AND NOT APPLE)
        target_link_libraries(opentrack-xplane-plugin rt)
    endif()
endif()

# some boilerplate

if(QT_USE_FILE)
    INCLUDE(${QT_USE_FILE})
endif()

add_library(opentrack-compat SHARED ${opentrack-compat-c})
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()

# hack to avoid breakage on buildbot
set(my-qt-deps)
if(WIN32)
    set(my-qt-deps ws2_32)
endif()

set(MY_QT_LIBS ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Xml_LIBRARIES} ${Qt5Core_LIBRARIES} ${my-qt-deps})

add_library(opentrack-csv SHARED ${opentrack-csv-c})
target_link_libraries(opentrack-csv ${MY_QT_LIBS})

add_library(opentrack-pose-widget SHARED ${opentrack-pose-widget-c} ${opentrack-pose-widget-rcc})
target_link_libraries(opentrack-pose-widget ${MY_QT_LIBS})
add_library(opentrack-spline-widget SHARED ${opentrack-spline-widget-c})
target_link_libraries(opentrack-spline-widget ${MY_QT_LIBS})

opentrack_library(opentrack-filter-accela)
opentrack_library(opentrack-filter-kalman)
opentrack_library(opentrack-filter-ewma)

target_link_libraries(opentrack-filter-kalman ${OpenCV_LIBS})

opentrack_library(opentrack-proto-fgfs)

if(SDK_VJOY)
    include_directories(${SDK_VJOY})
    opentrack_library(opentrack-proto-vjoy)
    if(MSVC)
        target_link_libraries(opentrack-proto-vjoy ${MY_QT_LIBS} "${SDK_VJOY}/VJoy.lib")
    else()
        target_link_libraries(opentrack-proto-vjoy ${MY_QT_LIBS} "${SDK_VJOY}/VJoy.dll")
    endif()
endif()

if(SDK_ENABLE_LIBEVDEV)
    opentrack_library(opentrack-proto-libevdev)
    target_link_libraries(opentrack-proto-libevdev evdev)
endif()

if(SDK_FSUIPC)
    if(MSVC)
            set(link-flags "/NODEFAULTLIB:libc")
    endif()
    opentrack_library(opentrack-proto-fsuipc LINK "${link-flags}")
    target_link_libraries(opentrack-proto-fsuipc "${SDK_FSUIPC}/FSUIPC_User.lib")
endif()

if(SDK_SIMCONNECT)
    opentrack_library(opentrack-proto-simconnect)
    include_directories("${SDK_SIMCONNECT}/inc")
    target_link_libraries(opentrack-proto-simconnect "${SDK_SIMCONNECT}/lib/SimConnect.lib")
endif()

if(WIN32)
    opentrack_library(opentrack-proto-freetrack)
    target_link_libraries(opentrack-proto-freetrack opentrack-csv opentrack-compat)
    opentrack_library(opentrack-proto-win32-mouse)
endif()

if(WIN32)
    add_library(freetrackclient SHARED ${opentrack-freetrack-c})
    if(CMAKE_COMPILER_IS_GNUCC)
        set_target_properties(freetrackclient PROPERTIES LINK_FLAGS "-Wl,--enable-stdcall-fixup -fno-lto -Wl,-kill-at" PREFIX "" COMPILE_FLAGS "-fno-lto")
    endif()
endif()

opentrack_library(opentrack-proto-udp)

if(WIN32)
    opentrack_library(opentrack-tracker-joystick)
endif()

if(SDK_WINE_PREFIX)
    opentrack_library(opentrack-proto-wine)
    target_link_libraries(opentrack-proto-wine opentrack-compat opentrack-csv)
    if(NOT SDK_WINE_NO_WRAPPER)
	set(my-rt -lrt)
	if(APPLE)
		set(my-rt)
	endif()
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
                  ${my-rt})
        add_custom_target(wine-wrapper ALL DEPENDS opentrack-wrapper-wine.exe.so)
        add_dependencies(wine-wrapper opentrack-compat opentrack-proto-wine)
    endif()
endif()

opentrack_library(opentrack-tracker-ht)
target_link_libraries(opentrack-tracker-ht opentrack-compat)

if(SDK_ARUCO_LIBPATH)
    include_directories(${CMAKE_SOURCE_DIR}/ftnoir_tracker_aruco/include)
    opentrack_library(opentrack-tracker-aruco)
    target_link_libraries(opentrack-tracker-aruco ${SDK_ARUCO_LIBPATH} ${OpenCV_LIBS})
    if(WIN32 AND MSVC)
        target_link_libraries(opentrack-tracker-aruco
                      "${CMAKE_SOURCE_DIR}/dinput/dxguid.lib"
                      "${CMAKE_SOURCE_DIR}/dinput/strmiids.lib"
                      uuid)
    endif()
endif()

link_with_dinput8(opentrack-tracker-ht)
link_with_dinput8(opentrack-tracker-joystick)

opentrack_library(opentrack-tracker-pt)
target_link_libraries(opentrack-tracker-pt ${OpenCV_LIBS})

link_with_dinput8(opentrack-tracker-pt)

opentrack_library(opentrack-tracker-udp)

if(SDK_RIFT)
    include_directories("${SDK_RIFT}/Include")
    include_directories("${SDK_RIFT}/Src")
    set(link-flags)
    set(c-flags)
    if(APPLE)
        set(link-flags "-framework CoreFoundation -framework CoreGraphics -framework IOKit -framework Quartz")
        set(c-flags "-fno-strict-aliasing")
    else()
        if(MSVC)
            set(link-flags "/NODEFAULTLIB:LIBCMT")
        else()
            set(c-flags "-fno-strict-aliasing")
        endif()
    endif()
    opentrack_library(opentrack-tracker-rift LINK "${link-flags}" COMPILE "${c-flags}")
    if(MSVC)
            target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/Lib/Win32/libovr.lib" winmm.lib setupapi.lib)
    else()
        if(WIN32)
            target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/libLibOVR.a" winmm setupapi)
        else()
            if(NOT APPLE)
                target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/libLibOVR.a" udev Xinerama)
            else()
                target_link_libraries(opentrack-tracker-rift "${SDK_RIFT}/libLibOVR.a")
            endif()
        endif()
    endif()
endif()

if(SDK_HYDRA)
    include_directories("${SDK_HYDRA}/include")
    include_directories("${SDK_HYDRA}/include/sixense_utils")
    opentrack_library(opentrack-tracker-hydra)
    if(WIN32)
        target_link_libraries(opentrack-tracker-hydra
            "${SDK_HYDRA}/lib/win32/release_dll/sixense.lib"
            "${SDK_HYDRA}/lib/win32/release_dll/sixense_utils.lib")
                    install(FILES "${SDK_HYDRA}/bin/win32/release_dll/sixense.dll" "${SDK_HYDRA}/bin/win32/release_dll/sixense_utils.dll" DESTINATION .)
            else()
                    if(SDK_HYDRA_AMD64)
                            set(underscore-sixtyfour _x64)
                    else()
                            set(underscore-sixtyfour)
                    endif()
                    if(APPLE)
                            set(underscore-dll _dll)
                            set(soext dylib)
                            set(sixense-plat osx)
                    else()
                            set(underscore-dll)
                            set(soext so)
                            set(sixense-plat linux)
                    endif()
                    install(FILES
                            "${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense${underscore-sixtyfour}.${soext}"
                            "${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense_utils${underscore-sixtyfour}.${soext}"
                            DESTINATION .
                    )
                    target_link_libraries(opentrack-tracker-hydra "${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense${underscore-sixtyfour}.${soext}" "${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense_utils${underscore-sixtyfour}.${soext}")
    endif()
endif()

if(WIN32 AND NOT SDK_CONSOLE_DEBUG)
    set(opentrack-win32-executable WIN32)
else()
    set(opentrack-win32-executable "")
endif()
if(UNIX OR APPLE)
    add_library(opentrack-qxt-mini SHARED ${qxt-mini-c})
    SET_TARGET_PROPERTIES(opentrack-qxt-mini PROPERTIES COMPILE_FLAGS "-DBUILD_QXT_CORE=42 -DBUILD_QXT_WIDGETS=42 -DBUILD_QXT_GUI=42")
    target_link_libraries(opentrack-qxt-mini ${MY_QT_LIBS})
    if(NOT APPLE)
	    target_link_libraries(opentrack-qxt-mini X11)
    endif()
endif()
add_executable(opentrack ${opentrack-win32-executable} ${opentrack-bin-c} ${opentrack-bin-uih} ${opentrack-bin-rcc})
set_target_properties(opentrack PROPERTIES COMPILE_DEFINITIONS OPENTRACK_VERSION=\"${OPENTRACK__COMMIT}\")
set(OPENTRACK_COMMIT_VERSION \"${OPENTRACK__COMMIT}\")
configure_file("${CMAKE_SOURCE_DIR}/opentrack-version.h" "${CMAKE_BINARY_DIR}/opentrack-version.h" @ONLY NEWLINE_STYLE UNIX)

if(APPLE)
    SET_TARGET_PROPERTIES(opentrack-qxt-mini PROPERTIES LINK_FLAGS "-framework Carbon -framework CoreFoundation")
endif()

add_library(opentrack-api SHARED ${opentrack-lib-c})
target_link_libraries(opentrack-api ${MY_QT_LIBS})
if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    SET_TARGET_PROPERTIES(opentrack-api PROPERTIES
        LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/opentrack-api/gnuc-version-script.txt"
        COMPILE_FLAGS "-fvisibility=protected  -fvisibility-inlines-hidden"
        COMPILE_DEFINITIONS IN_OPENTRACK_API
    )
endif()

set_target_properties(opentrack PROPERTIES COMPILE_DEFINITIONS OPENTRACK_VERSION=\"${OPENTRACK__COMMIT}\")

if(UNIX OR APPLE)
    target_link_libraries(opentrack opentrack-qxt-mini)
endif()
target_link_libraries(opentrack ${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})

if(UNIX OR APPLE)
    install(TARGETS opentrack-qxt-mini RUNTIME DESTINATION . LIBRARY DESTINATION .  )
endif()

link_with_dinput8(opentrack)

if(CMAKE_SYSTEM STREQUAL LINUX)
    link_libraries(rt)
endif()

if(MSVC)
    SET_TARGET_PROPERTIES(opentrack PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()
target_link_libraries(opentrack opentrack-pose-widget opentrack-spline-widget ${MY_QT_LIBS} ${QXT_QXTCORE_LIB_RELEASE} ${QXT_QXTWIDGETS_LIB_RELEASE})
if(NOT WIN32)
    target_link_libraries(opentrack dl)
    target_link_libraries(opentrack-api dl)
endif()
if(SDK_GOOGLE_BREAKPAD)
    if(MSVC)
        target_link_libraries(opentrack
            "${SDK_GOOGLE_BREAKPAD}/src/client/windows/Release/lib/crash_generation_client.lib"
            "${SDK_GOOGLE_BREAKPAD}/src/client/windows/Release/lib/exception_handler.lib"
            "${SDK_GOOGLE_BREAKPAD}/src/client/windows/Release/lib/common.lib")
    endif()
endif()
set_target_properties(opentrack PROPERTIES COMPILE_FLAGS -DOPENTRACK_MAIN)

# make install

install(FILES "${CMAKE_SOURCE_DIR}/README.md" DESTINATION .)

if(SDK_XPLANE)
    install(TARGETS opentrack-xplane-plugin RUNTIME DESTINATION . LIBRARY DESTINATION .  )
endif()

if(WIN32)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tracker-ht" DESTINATION .)
    install(TARGETS freetrackclient RUNTIME DESTINATION . LIBRARY DESTINATION .  )
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/3rdparty-notices" DESTINATION .)

install(FILES "${CMAKE_SOURCE_DIR}/bin/NPClient.dll" "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll" "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe" DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/settings" "${CMAKE_SOURCE_DIR}/facetracknoir/clientfiles" DESTINATION .)

if(NOT WIN32 AND SDK_WINE_PREFIX)
    install(FILES "${CMAKE_BINARY_DIR}/opentrack-wrapper-wine.exe.so"
            DESTINATION .)
endif()

install(TARGETS
        opentrack-api
        opentrack-compat
        opentrack-csv
        opentrack-pose-widget
        opentrack-spline-widget
        RUNTIME DESTINATION . LIBRARY DESTINATION .  )

install(TARGETS opentrack DESTINATION .)

if(SDK_VJOY)
    install(FILES "${SDK_VJOY}/VJoy.dll" DESTINATION .)
endif()

if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
endif()

if(MSVC)
    file(GLOB pdbs1 "${CMAKE_BINARY_DIR}/Release/*.pdb")
    file(GLOB pdbs2 "${CMAKE_BINARY_DIR}/*.pdb")
    install(FILES ${pdbs1} ${pdbs2} DESTINATION .)
endif()
