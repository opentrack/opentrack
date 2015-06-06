# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  i686-w64-mingw32-windres)
set(CMAKE_LINKER  i686-w64-mingw32-ld)

SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(lto "-flto -flto-partition=none -fno-use-linker-plugin")
set(rice2 "-ftree-vectorize -fivopts -fweb")
set(rice3 "-ffast-math -mfpmath=both -frename-registers -funsafe-loop-optimizations")
set(rice4 "-fgcse-sm -fgcse-las -fgcse-after-reload")
set(rice5 "-fmodulo-sched -fmodulo-sched-allow-regmoves")
set(rice "${rice2} ${rice3} ${rice4} ${rice5}")
set(CMAKE_C_FLAGS_RELEASE "-O3  ${lto} ${rice} -march=i686 -mtune=corei7-avx -msse -msse2 -mno-sse3 -mno-avx" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-msse -msse2 -mno-sse3 -mno-avx ${lto}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
