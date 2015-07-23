project(opentrack)
cmake_minimum_required(VERSION 2.8.11)
cmake_policy(SET CMP0020 NEW)
if(POLICY CMP0058)
    cmake_policy(SET CMP0058 OLD)
endif()
if(POLICY CMP0028)
    cmake_policy(SET CMP0028 OLD)
endif()

include(CMakeParseArguments)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include(GetGitRevisionDescription)
find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK__COMMIT --tags --always)
endif()

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_INSTALL_RPATH FALSE)
SET(CMAKE_SKIP_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenCV 3.0)

find_package(Qt5 REQUIRED COMPONENTS Core Xml Network Widgets Gui QUIET)
find_package(Qt5 COMPONENTS SerialPort QUIET)
include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Xml_DEFINITIONS} ${Qt5Gui_DEFINITIONS} ${Qt5Widgets_DEFINITIONS} ${Qt5Network_DEFINITIONS})

if(MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

set(my-qt-deps)
if(WIN32) # hack to avoid breakage on buildbot
    set(my-qt-deps ws2_32)
endif()
set(MY_QT_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Xml_LIBRARIES} ${my-qt-deps})

# note, hatire supports both ftnoir and opentrack
# don't remove without being sure as hell -sh 20140922
add_definitions(-DOPENTRACK_API)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
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
    set(CMAKE_MACOSX_RPATH OFF)
    set(apple-frameworks "-stdlib=libc++ -framework Cocoa -framework CoreFoundation -lobjc -lz -framework Carbon")
    set(CMAKE_SHARED_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_SHARED_LINKER_FLAGS}")
    #set(CMAKE_STATIC_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS " -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    set(CMAKE_CXX_FLAGS " -std=c++11 ${CMAKE_CXX_FLAGS} ")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS OFF)

macro(opentrack_module n dir)
    file(GLOB ${n}-c ${dir}/*.cpp ${dir}/*.c ${dir}/*.h ${dir}/*.hpp)
    file(GLOB ${n}-res ${dir}/*.rc)
    foreach(f ${n}-res)
        set_source_files_properties(${f} PROPERTIES LANGUAGE RC)
    endforeach()
    file(GLOB ${n}-ui ${dir}/*.ui)
    file(GLOB ${n}-rc ${dir}/*.qrc)
endmacro()

macro(opentrack_qt n)
    qt5_wrap_cpp(${n}-moc ${${n}-c} OPTIONS --no-notes)
    QT5_WRAP_UI(${n}-uih ${${n}-ui})
    QT5_ADD_RESOURCES(${n}-rcc ${${n}-rc})
    set(${n}-all ${${n}-c} ${${n}-rc} ${${n}-rcc} ${${n}-uih} ${${n}-moc} ${${n}-res})
endmacro()

set(msvc-subsystem "/VERSION:5.1 /SUBSYSTEM:WINDOWS,5.01")
function(opentrack_compat target)
    if(MSVC)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "${msvc-subsystem} /DEBUG /OPT:ICF")
    endif()
endfunction()

macro(opentrack_library n dir)
    cmake_parse_arguments(foolib "" "LINK;COMPILE;GNU-LINK;GNU-COMPILE;STATIC" "" ${ARGN})
    if(NOT " ${foolib_UNPARSED_ARGUMENTS}" STREQUAL " ")
        message(FATAL_ERROR "opentrack_library bad formals")
    endif()
    opentrack_module(${n} ${dir})
    opentrack_qt(${n})
    set(link-mode SHARED)
    if (foolib_STATIC)
        set(link-mode STATIC)
    endif()
    add_library(${n} ${link-mode} ${${n}-all})
    target_link_libraries(${n} opentrack-api ${MY_QT_LIBS})
    if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
        SET_TARGET_PROPERTIES(${n} PROPERTIES
            LINK_FLAGS "${foolib_LINK} ${foolib_GNU-LINK} -Wl,--as-needed -Wl,--version-script=\"${CMAKE_SOURCE_DIR}/opentrack/${version-script}-version-script.txt\""
            COMPILE_FLAGS "${foolib_COMPILE} ${foolib_GNU-COMPILE} -fvisibility=hidden -fvisibility-inlines-hidden"
        )
    else()
        set(link-flags)
        if(MSVC)
            set(link-flags "${msvc-subsystem} /DEBUG /OPT:ICF")
        endif()
        set_target_properties(${n} PROPERTIES LINK_FLAGS "${link-flags} ${foolib_LINK}" COMPILE_FLAGS "${foolib_COMPILE}")
    endif()
    install(TARGETS ${n} RUNTIME DESTINATION . LIBRARY DESTINATION .)
endmacro()

function(link_with_dinput8 n)
    if(WIN32)
        target_link_libraries(${n} dinput8 dxguid strmiids)
    endif()
endfunction()

# ----

# cache variables

# ----

IF(WIN32)
    SET(SDK_VJOY "" CACHE PATH "VJoy SDK path")
    SET(SDK_CONSOLE_DEBUG FALSE CACHE BOOL "Console window visible at runtime")
ENDIF()

IF("${CMAKE_SYSTEM}" MATCHES "Linux" OR APPLE)
    set(SDK_XPLANE "" CACHE PATH "Path to X-Plane SDK")
    set(SDK_ENABLE_LIBEVDEV FALSE CACHE BOOL "libevdev virtual joystick protocol support")
endif()

SET(SDK_HYDRA "" CACHE PATH "libSixense path for Razer Hydra")
SET(SDK_HYDRA_AMD64 FALSE CACHE BOOL "whether libSixense is amd64 (else ia-32)")
SET(SDK_RIFT "" CACHE PATH "libOVR path for Oculus Rift")

set(SDK_ARUCO_LIBPATH "" CACHE FILEPATH "Aruco paper marker tracker static library path")

set(SDK_HT "" CACHE FILEPATH "Path to headtracker library")
set(SDK_HT_FLANDMARK "" CACHE FILEPATH "Path to flandmark library for headtracker")

if(WIN32)
    set(SDK_FSUIPC "" CACHE PATH "FSUIPC for older MS FSX path")
endif()

if(NOT WIN32)
    set(SDK_WINE_PREFIX "" CACHE PATH "Wine install prefix")
    set(SDK_WINE_NO_WRAPPER FALSE CACHE BOOL "disable Wine wrapper -- use Wine only for X-Plane")
endif()

# ----

# misc

# ----

string(TIMESTAMP filename-date "%Y%m%d")
set(filename-ostype ${CMAKE_SYSTEM_NAME})
get_git_head_revision(filename-branch_0 filename-hash_0)
if(filename-hash_0)
    string(SUBSTRING "${filename-hash_0}" 0 7 filename-hash)
endif()
string(REPLACE "refs/heads/" "" filename-branch_1 "${filename-branch_0}")
string(REPLACE "/" "-" filename-branch "${filename-branch_1}")
set(filename_0 "${OPENTRACK__COMMIT}")
set(filename "${CMAKE_BINARY_DIR}/${filename_0}.zip")

add_custom_command(OUTPUT ${filename} COMMAND env sh "${CMAKE_SOURCE_DIR}/make-tar.sh" "${CMAKE_INSTALL_PREFIX}" "${filename}")
add_custom_target(tarball DEPENDS ${filename})


opentrack_module(opentrack-api opentrack)
opentrack_qt(opentrack-api)
add_library(opentrack-api STATIC ${opentrack-api-all})
opentrack_compat(opentrack-api)
target_link_libraries(opentrack-api ${MY_QT_LIBS})

if(NOT WIN32)
    target_link_libraries(opentrack-api dl)
else()
    target_link_libraries(opentrack-api winmm)
endif()

# ----

# conditional targets

# ----

set(opentrack-perms PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> --use-temp-file -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  ENABLE_LANGUAGE(RC)
endif(WIN32)

opentrack_module(opentrack-compat compat)
opentrack_module(opentrack-xplane-plugin x-plane-plugin)

if(SDK_XPLANE)
    # probably librt already included
    add_library(opentrack-xplane-plugin SHARED ${opentrack-xplane-plugin-c})
    target_include_directories(opentrack-xplane-plugin SYSTEM PUBLIC ${SDK_XPLANE}/CHeaders ${SDK_XPLANE}/CHeaders/XPLM)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
        SET_TARGET_PROPERTIES(opentrack-xplane-plugin
            PROPERTIES LINK_FLAGS
            "-Wl,--version-script=${CMAKE_SOURCE_DIR}/x-plane-plugin/version-script.txt -shared -rdynamic -nodefaultlibs -undefined_warning -fPIC"
            COMPILE_FLAGS "-Wall -O2 -pipe -fPIC -DLIN -DXPLM200 -DXPLM210"
            LIBRARY_OUTPUT_NAME "opentrack.xpl"
            PREFIX "" SUFFIX "")
    endif()
    if(APPLE)
        SET_TARGET_PROPERTIES(opentrack-xplane-plugin PROPERTIES
                              COMPILE_FLAGS "-iframework ${SDK_XPLANE}/Libraries/Mac/ -DAPL -DXPLM200 -DXPLM210 -framework XPLM -framework XPWidgets"
                              LINK_FLAGS "-F${SDK_XPLANE}/Libraries/Mac/ -framework XPLM -framework XPWidgets")
    endif()
    if(UNIX AND NOT APPLE)
        target_link_libraries(opentrack-xplane-plugin rt)
    endif()
endif()

add_library(opentrack-compat STATIC ${opentrack-compat-c})
opentrack_compat(opentrack-compat) # uh...
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()

opentrack_module(opentrack-csv csv)
add_library(opentrack-csv STATIC ${opentrack-csv-c})
opentrack_compat(opentrack-csv)
target_link_libraries(opentrack-csv ${MY_QT_LIBS})

opentrack_module(opentrack-pose-widget pose-widget)
opentrack_qt(opentrack-pose-widget)

add_library(opentrack-pose-widget STATIC ${opentrack-pose-widget-all})
opentrack_compat(opentrack-pose-widget)
target_include_directories(opentrack-pose-widget PUBLIC pose-widget/) # else Qt moc breaks
target_link_libraries(opentrack-pose-widget ${MY_QT_LIBS})

opentrack_module(opentrack-spline-widget qfunctionconfigurator)
opentrack_qt(opentrack-spline-widget)
add_library(opentrack-spline-widget STATIC ${opentrack-spline-widget-all})
opentrack_compat(opentrack-spline-widget)
target_include_directories(opentrack-spline-widget PUBLIC qfunctionconfigurator/)
target_link_libraries(opentrack-spline-widget ${MY_QT_LIBS})

add_library(opentrack-version STATIC opentrack/version.cc)
opentrack_compat(opentrack-version)
set_target_properties(opentrack-version PROPERTIES
    COMPILE_DEFINITIONS
    "OPENTRACK_VERSION=\"${OPENTRACK__COMMIT}\"")

opentrack_library(opentrack-filter-accela ftnoir_filter_accela)
target_link_libraries(opentrack-filter-accela opentrack-spline-widget)
opentrack_library(opentrack-filter-ewma ftnoir_filter_ewma2)

opentrack_library(opentrack-proto-fgfs ftnoir_protocol_fg)

if(SDK_VJOY)
    opentrack_library(opentrack-proto-vjoy ftnoir_protocol_vjoy GNU-LINK "-Wl,--enable-stdcall-fixup")
    if(MSVC)
        set(ext .lib)
    else()
        set(ext .dll)
    endif()
    target_link_libraries(opentrack-proto-vjoy ${MY_QT_LIBS} ${SDK_VJOY}/VJoy${ext})
    target_include_directories(opentrack-proto-vjoy SYSTEM PUBLIC ${SDK_VJOY})
endif()

if(SDK_ENABLE_LIBEVDEV)
    include(FindPkgConfig)
    opentrack_library(opentrack-proto-libevdev ftnoir_protocol_libevdev)
    pkg_check_modules(libevdev REQUIRED QUIET libevdev)
    target_link_libraries(opentrack-proto-libevdev ${libevdev_LIBRARIES})
    include_directories(opentrack-proto-libevdev SYSTEM PUBLIC ${libevdev_INCLUDE_DIRS})
endif()

if(SDK_FSUIPC)
    opentrack_library(opentrack-proto-fsuipc ftnoir_protocol_fsuipc)
    target_link_libraries(opentrack-proto-fsuipc ${SDK_FSUIPC}/FSUIPC_User.lib)
    target_include_directories(opentrack-proto-fsuipc SYSTEM PUBLIC ${SDK_FSUIPC})
    if(MSVC)
        set_target_properties(opentrack-proto-fsuipc PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBC.lib")
    endif()
    #target_link_directories(${SDK_FSUIPC})
endif()

if(WIN32)
    opentrack_library(opentrack-proto-simconnect ftnoir_protocol_sc)
endif()

if(WIN32)
    opentrack_library(opentrack-proto-freetrack ftnoir_protocol_ft)
    target_link_libraries(opentrack-proto-freetrack opentrack-csv opentrack-compat)
    opentrack_library(opentrack-proto-win32-mouse ftnoir_protocol_mouse)
endif()

opentrack_library(opentrack-proto-udp ftnoir_protocol_ftn)

if(WIN32)
    opentrack_library(opentrack-tracker-joystick ftnoir_tracker_joystick)
endif()

if(Qt5SerialPort_FOUND)
    opentrack_library(opentrack-tracker-hatire ftnoir_tracker_hatire)
    target_link_libraries(opentrack-tracker-hatire ${Qt5SerialPort_LIBRARIES})
    target_include_directories(opentrack-tracker-hatire SYSTEM PUBLIC ${Qt5SerialPort_INCLUDE_DIRS})
endif()

if(SDK_WINE_PREFIX)
    opentrack_library(opentrack-proto-wine ftnoir_protocol_wine)
    target_link_libraries(opentrack-proto-wine opentrack-compat opentrack-csv)
    if(NOT SDK_WINE_NO_WRAPPER)
        set(my-rt -lrt)
        if(APPLE)
            set(my-rt)
        endif()
        file(GLOB wine-deps ${CMAKE_SOURCE_DIR}/ftnoir_protocol_wine/*.cxx)
        add_custom_command(
          OUTPUT opentrack-wrapper-wine.exe.so
          DEPENDS ${wine-deps}
          COMMAND ${SDK_WINE_PREFIX}/bin/wineg++ -g -DNOMINMAX -O2 -m32 -std=c++11 -o
                  opentrack-wrapper-wine.exe -I "${CMAKE_SOURCE_DIR}"
                  ${wine-deps}
                  ${my-rt})
        add_custom_target(wine-wrapper ALL DEPENDS opentrack-wrapper-wine.exe.so)
        add_dependencies(opentrack-proto-wine wine-wrapper)
        add_dependencies(wine-wrapper opentrack-compat)
    endif()
endif()

opentrack_library(opentrack-tracker-udp ftnoir_tracker_udp)
opentrack_library(opentrack-tracker-freepie-udp ftnoir_tracker_freepie-udp)

if(OpenCV_FOUND)
    opentrack_library(opentrack-tracker-pt ftnoir_tracker_pt)
    target_link_libraries(opentrack-tracker-pt ${OpenCV_LIBS})
    target_include_directories(opentrack-tracker-pt SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
    link_with_dinput8(opentrack-tracker-pt)

    if(SDK_ARUCO_LIBPATH)
        opentrack_library(opentrack-tracker-aruco ftnoir_tracker_aruco)
        target_link_libraries(opentrack-tracker-aruco ${SDK_ARUCO_LIBPATH} ${OpenCV_LIBS})
        target_include_directories(opentrack-tracker-aruco SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
        link_with_dinput8(opentrack-tracker-aruco)
    endif()

    opentrack_library(opentrack-filter-kalman ftnoir_filter_kalman)
    target_link_libraries(opentrack-filter-kalman ${OpenCV_LIBS})
    target_include_directories(opentrack-filter-kalman SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})

    if(SDK_HT AND SDK_HT_FLANDMARK)
        opentrack_library(opentrack-tracker-ht ftnoir_tracker_ht)
        target_link_libraries(opentrack-tracker-ht opentrack-compat ${SDK_HT} ${SDK_HT_FLANDMARK} ${OpenCV_LIBS})
        link_with_dinput8(opentrack-tracker-ht)
        target_include_directories(opentrack-tracker-ht SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
    endif()
endif()

link_with_dinput8(opentrack-tracker-joystick)

if(SDK_RIFT)
    set(link-flags)
    set(c-flags)
    if(APPLE)
        set(link-flags "-framework CoreFoundation -framework CoreGraphics -framework IOKit -framework Quartz")
        set(c-flags "-fno-strict-aliasing")
    else()
		if(NOT MSVC)
			set(c-flags "-fno-strict-aliasing")
		endif()
    endif()
    opentrack_library(opentrack-tracker-rift ftnoir_tracker_rift LINK ${link-flags} COMPILE ${c-flags})
    target_include_directories(opentrack-tracker-rift SYSTEM PUBLIC ${SDK_RIFT}/Include ${SDK_RIFT}/Src)
    if(WIN32)
        if(MSVC)
            set(ext lib)
            set(p)
        else()
            set(ext a)
            set(p lib)
        endif()
        target_link_libraries(opentrack-tracker-rift ${SDK_RIFT}/${p}LibOVR.${ext} winmm setupapi ws2_32 imagehlp wbemuuid)
    else()
        if(NOT APPLE)
                target_link_libraries(opentrack-tracker-rift ${SDK_RIFT}/libLibOVR.a udev Xinerama)
        else()
                target_link_libraries(opentrack-tracker-rift ${SDK_RIFT}/libLibOVR.a)
        endif()
    endif()
endif()

if(SDK_HYDRA)
    opentrack_library(opentrack-tracker-hydra ftnoir_tracker_hydra)
    target_include_directories(opentrack-tracker-hydra SYSTEM PUBLIC ${SDK_HYDRA}/include ${SDK_HYDRA}/include/sixense_utils)
    if(WIN32)
        if(MSVC)
            set(dir lib)
            set(ext lib)
        else()
            set(dir bin)
            set(ext dll)
        endif()
        target_link_libraries(opentrack-tracker-hydra
                              "${SDK_HYDRA}/${dir}/win32/release_dll/sixense.${ext}"
                              #"${SDK_HYDRA}/${dir}/win32/release_dll/sixense_utils.${ext}"
                             )
                    install(FILES "${SDK_HYDRA}/bin/win32/release_dll/sixense.dll"
                                  #"${SDK_HYDRA}/bin/win32/release_dll/sixense_utils.dll"
                            DESTINATION . PERMISSIONS WORLD_READ WORLD_EXECUTE OWNER_WRITE OWNER_READ OWNER_EXECUTE)
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
                            #"${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense_utils${underscore-sixtyfour}.${soext}"
                            DESTINATION .
                    )
                    target_link_libraries(opentrack-tracker-hydra
                                          "${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense${underscore-sixtyfour}.${soext}"
                                          #"${SDK_HYDRA}/lib/${sixense-plat}${underscore-sixtyfour}/release${underscore-dll}/libsixense_utils${underscore-sixtyfour}.${soext}"
                                         )
    endif()
endif()

if(UNIX OR APPLE)
    # Qxt bundled :: sorry for this, but gentoo ebuild is broken
    include_directories("qxt-mini/")
    if(APPLE)
        set(qxt-plat mac)
    else()
        set(qxt-plat x11)
    endif()
    file(GLOB qxt-mini-c qxt-mini/*.h qxt-mini/qxtglobalshortcut.cpp qxt-mini/plat/qxtglobalshortcut_${qxt-plat}.cpp)
    opentrack_qt(qxt-mini)
    add_library(opentrack-qxt-mini STATIC ${qxt-mini-all})
    target_link_libraries(opentrack-qxt-mini ${MY_QT_LIBS})
    if(NOT APPLE)
            target_link_libraries(opentrack-qxt-mini X11)
    endif()
endif()

if(WIN32 AND NOT SDK_CONSOLE_DEBUG)
    set(opentrack-win32-executable WIN32)
else()
    set(opentrack-win32-executable "")
endif()

opentrack_module(opentrack facetracknoir)
if(UNIX OR APPLE)
    list(APPEND opentrack-c qxt-mini/qxtglobalshortcut.h)
endif()
opentrack_qt(opentrack)
add_executable(opentrack ${opentrack-win32-executable} ${opentrack-all})
opentrack_compat(opentrack)
if(NOT WIN32)
    set_target_properties(opentrack PROPERTIES SUFFIX ".bin")
endif()
target_link_libraries(opentrack opentrack-api opentrack-version opentrack-pose-widget opentrack-spline-widget)

if(APPLE)
    SET_TARGET_PROPERTIES(opentrack-qxt-mini PROPERTIES LINK_FLAGS "-framework Carbon -framework CoreFoundation")
endif()

if(UNIX OR APPLE)
    target_link_libraries(opentrack opentrack-qxt-mini)
endif()

link_with_dinput8(opentrack)
target_link_libraries(opentrack ${MY_QT_LIBS})

if(CMAKE_SYSTEM STREQUAL LINUX)
    link_libraries(rt)
endif()

# ----

# make install

# ----

install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION .)

if(SDK_XPLANE)
    install(TARGETS opentrack-xplane-plugin RUNTIME DESTINATION . LIBRARY DESTINATION .  )
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty-notices DESTINATION .)

install(FILES "${CMAKE_SOURCE_DIR}/bin/freetrackclient.dll" DESTINATION . ${opentrack-perms})
install(FILES "${CMAKE_SOURCE_DIR}/bin/NPClient.dll" "${CMAKE_SOURCE_DIR}/bin/NPClient64.dll" "${CMAKE_SOURCE_DIR}/bin/TrackIR.exe" DESTINATION .
    ${opentrack-perms})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/settings" "${CMAKE_SOURCE_DIR}/clientfiles" DESTINATION .)

if(NOT WIN32 AND SDK_WINE_PREFIX AND NOT SDK_WINE_NO_WRAPPER)
    install(FILES "${CMAKE_BINARY_DIR}/opentrack-wrapper-wine.exe.so"
            DESTINATION .)
endif()

install(TARGETS opentrack DESTINATION .)

if(SDK_VJOY)
    install(FILES "${SDK_VJOY}/VJoy.dll" DESTINATION . ${opentrack-perms})
endif()

if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/bin/cleye.config" DESTINATION .)
endif()

if(APPLE)
    install(CODE "
    execute_process(COMMAND /bin/sh \"${CMAKE_SOURCE_DIR}/macosx/make-app-bundle.sh\"
                                    \"${CMAKE_SOURCE_DIR}/macosx\"
                                    \"${CMAKE_INSTALL_PREFIX}\"
                                    \"${CMAKE_BINARY_DIR}\"
                                    \"${OPENTRACK__COMMIT}\")
    ")
endif()
