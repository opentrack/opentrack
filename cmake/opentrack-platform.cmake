IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> --use-temp-file -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  ENABLE_LANGUAGE(RC)
endif(WIN32)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# note, hatire supports both ftnoir and opentrack
# don't remove without being sure as hell -sh 20140922
add_definitions(-DOPENTRACK_API)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
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

# nix -rdynamic passed from Linux-GNU.cmake
if(CMAKE_COMPILER_IS_GNUCXX)
    set(__LINUX_COMPILER_GNU 1)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
endif()
