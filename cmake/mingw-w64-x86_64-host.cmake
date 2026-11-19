# Cross-compilation toolchain: build Windows x86_64 binaries from a Unix host
# with a mingw-w64 toolchain in PATH (e.g. Homebrew's mingw-w64 on macOS).
# Usage:
#   cmake -GNinja -S . -B build-win \
#       -DCMAKE_TOOLCHAIN_FILE=$(pwd)/cmake/mingw-w64-x86_64-host.cmake \
#       -DQt6_DIR=/path/to/qt-win/lib/cmake/Qt6 \
#       -DQT_HOST_PATH=/path/to/host/qt
# Derived from mingw-w64.cmake in this directory.

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "" FORCE)
endif()

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 10)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)

set(p "")
set(e "")
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(e .exe)
endif()

SET(CMAKE_C_COMPILER    ${p}x86_64-w64-mingw32-gcc${e})
SET(CMAKE_CXX_COMPILER  ${p}x86_64-w64-mingw32-g++${e})
set(CMAKE_RC_COMPILER   ${p}x86_64-w64-mingw32-windres${e})
set(CMAKE_LINKER        ${p}x86_64-w64-mingw32-ld${e})
set(CMAKE_AR            ${p}x86_64-w64-mingw32-gcc-ar${e})
set(CMAKE_NM            ${p}x86_64-w64-mingw32-gcc-nm${e})
set(CMAKE_RANLIB        ${p}x86_64-w64-mingw32-gcc-ranlib${e})
set(CMAKE_OBJCOPY       ${p}x86_64-w64-mingw32-objcopy${e})
set(CMAKE_OBJDUMP       ${p}x86_64-w64-mingw32-objdump${e})

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(fpu "-ffast-math -mfpmath=sse -mstackrealign")
set(cpu "-O3 -msse3 -mtune=skylake")
set(sections "-ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS_RELEASE "${cpu} ${fpu} ${sections}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${cpu} ${fpu} ${sections}" CACHE STRING "" FORCE)

add_definitions(-DSTRSAFE_NO_DEPRECATE)
