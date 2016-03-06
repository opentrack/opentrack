# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
set(p c:/mingw-w64/i686-5.3.0-posix-dwarf-rt_v4-rev0/mingw32/bin)
set(c ${p}/i686-w64-mingw32-)
set(CMAKE_MAKE_PROGRAM ${p}/mingw32-make.exe CACHE FILEPATH "" FORCE)

set(e .exe)

SET(CMAKE_C_COMPILER    ${c}gcc${e})
SET(CMAKE_CXX_COMPILER  ${c}g++${e})
set(CMAKE_RC_COMPILER   ${c}windres${e})
set(CMAKE_LINKER        ${c}ld${e})
set(CMAKE_AR            ${c}gcc-ar${e}      CACHE STRING "" FORCE)
set(CMAKE_NM            ${c}gcc-nm${e}      CACHE STRING "" FORCE)
set(CMAKE_RANLIB        ${c}gcc-ranlib${e}  CACHE STRING "" FORCE)

SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(fpu "-ffast-math -fno-finite-math-only -mfpmath=both")
set(cpu "-O3 -march=pentium4 -mtune=corei7-avx ${fpu} -msse -msse2 -mno-sse3 -frename-registers")

set(CFLAGS-OVERRIDE "" CACHE STRING "")

set(CMAKE_C_FLAGS_RELEASE "${cpu} ${CFLAGS-OVERRIDE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${cpu} ${CFLAGS-OVERRIDE}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
