# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
set(c i686-w64-mingw32-)

SET(CMAKE_C_COMPILER    ${c}gcc)
SET(CMAKE_CXX_COMPILER  ${c}g++)
set(CMAKE_RC_COMPILER   ${c}windres)
set(CMAKE_LINKER        ${c}ld)
set(CMAKE_AR            ${c}gcc-ar      CACHE STRING "" FORCE)
set(CMAKE_NM            ${c}gcc-nm      CACHE STRING "" FORCE)
set(CMAKE_RANLIB        ${c}gcc-ranlib  CACHE STRING "" FORCE)

SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(cpu "-O3 -march=i686 -mtune=corei7-avx -ffast-math -mfpmath=both -msse -msse2 -mno-sse3 -mno-avx")

set(CFLAGS-OVERRIDE "" CACHE STRING "")

set(CMAKE_C_FLAGS_RELEASE "${rice} ${cpu} ${CFLAGS-OVERRIDE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${cpu} ${CFLAGS-OVERRIDE}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
