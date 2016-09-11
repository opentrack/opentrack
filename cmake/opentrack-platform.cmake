if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_GNUC TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if((NOT CMAKE_COMPILER_IS_GNUCXX) EQUAL (NOT (NOT CMAKE_COMPILER_IS_GNUCC)))
    message(FATAL_ERROR "cannot use GNU C xor GNU C++")
endif()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0 -D_HAS_ITERATOR_DEBUGGING=0 -D_SECURE_SCL=0)
    set(CMAKE_CXX_FLAGS " /std:c++14 -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS ${CMAKE_CXX_FLAGS} ")
    set(CMAKE_C_FLAGS " -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS ${CMAKE_C_FLAGS} ")

    foreach (i SHARED MODULE EXE)
        set(CMAKE_${i}_LINKER_FLAGS " /DYNAMICBASE /NXCOMPAT ${CMAKE_${i}_LINKER_FLAGS} ")
    endforeach()
endif()

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> --use-temp-file -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  enable_language(RC)
  add_definitions(-D_USE_MATH_DEFINES=1)
endif(WIN32)

if(opentrack-install-rpath)
    set(CMAKE_INSTALL_RPATH "${opentrack-install-rpath}")
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_SOURCE_DIR})

if(APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
    set(apple-frameworks "-stdlib=libc++ -framework Cocoa -framework CoreFoundation -lobjc -lz -framework Carbon")
    set(CMAKE_SHARED_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_SHARED_LINKER_FLAGS}")
    #set(CMAKE_STATIC_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS " -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS " -std=c++14 ${CMAKE_CXX_FLAGS} ")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS OFF)

# nix -rdynamic passed from Linux-GNU.cmake
if(CMAKE_COMPILER_IS_GNUCXX)
    set(__LINUX_COMPILER_GNU 1)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
endif()

if(MINGW)
    add_definitions(-DMINGW_HAS_SECURE_API)
endif()

set(opencv-modules opencv_calib3d opencv_core opencv_features2d opencv_imgcodecs opencv_imgproc opencv_objdetect opencv_video opencv_videoio)
