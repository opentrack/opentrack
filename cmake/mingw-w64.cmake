# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
set(p c:/mingw-w64/i686-5.3.0-posix-dwarf-rt_v4-rev0/mingw32/bin)
set(c ${p}/i686-w64-mingw32-)
#set(CMAKE_MAKE_PROGRAM ${p}/mingw32-make.exe CACHE FILEPATH "" FORCE)

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
set(fpu "-ffast-math -fno-finite-math-only -mfpmath=both -mstackrealign -ftree-vectorize")
set(cpu "-O3 -march=pentium4 -mtune=corei7-avx -msse -msse2 -mno-sse3 -mno-avx -frename-registers -fno-PIC")
set(lto "-flto -fuse-linker-plugin")

set(_CFLAGS " -fvisibility=hidden ")
set(_CXXFLAGS " ${_CFLAGS} ")
set(_CFLAGS_RELEASE " -s ${cpu} ${fpu} ${lto} ")
set(_CFLAGS_DEBUG "-g -ggdb")
set(_CXXFLAGS_RELEASE " ${_CFLAGS_RELEASE}")
set(_CXXFLAGS_DEBUG " ${_CFLAGS_DEBUG} ")

set(_LDFLAGS " -Wl,--as-needed ${_CXXFLAGS} ")
set(_LDFLAGS_RELEASE " ${_CXXFLAGS_RELEASE} ")
set(_LDFLAGS_DEBUG " ${_CXXFLAGS_DEBUG} ")

set(WARNINGS_ENABLE TRUE CACHE BOOL "Emit additional warnings at compile-time")
# these are very noisy, high false positive rate. only for development.
set(WARNINGS_FINAL_SUGGESTIONS FALSE CACHE BOOL "Emit very noisy warnings at compile-time")
set(WARNINGS_PEDANTIC FALSE CACHE BOOL "Emit very noisy warnings at compile-time")
set(WARNINGS_NUMERIC FALSE CACHE BOOL "Emit very noisy warnings at compile-time")

set(noisy-warns "")
set(suggest-final "")
set(pedantics "")
set(numerics "")
if(CMAKE_PROJECT_NAME STREQUAL "opentrack" AND WARNINGS_ENABLE)
    if(WARNINGS_FINAL_SUGGESTIONS)
        set(suggest-final "-Wsuggest-final-types -Wsuggest-final-methods")
    endif()
    if(WARNINGS_PEDANTIC)
        set(pedantics "-Wuseless-cast -Wsuggest-override")
    endif()
    if(WARNINGS_NUMERIC)
        set(numerics "-Wdouble-promotion")
    endif()
    set(noisy-warns "${suggest-final} ${pedantics} ${numerics}")
endif()

# -Wodr and -Wattributes are disabled due to LTO false positives in dependencies.
set(clang-warns "")
if(CMAKE_COMPILER_IS_CLANG)
    set(clang-warns "-Wweak-vtables")
endif()
set(_CXX_WARNS "")
if(WARNINGS_ENABLE)
    set(_CXX_WARNS "-Wall -Wextra -pedantic -Wdelete-non-virtual-dtor -Wsign-compare -Wno-odr -Wno-attributes -Wcast-align -Wc++14-compat ${clang-warns} ${noisy-warns}")
endif()

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        set(OVERRIDE_${j}_FLAGS${i} "" CACHE STRING "")
        set(CMAKE_${j}_FLAGS${i} " ${_${j}FLAGS${i}} ${_${j}_WARNS} ${OVERRIDE_${j}_FLAGS${i}} " CACHE STRING "" FORCE)
    endforeach()
endforeach()

foreach (i "" _DEBUG _RELEASE)
    set(CMAKE_CXX_FLAGS${i} " ${CMAKE_CXX_FLAGS${i}} " CACHE STRING "" FORCE)
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        set(OVERRIDE_LDFLAGS${j} "" CACHE STRING "")
        set(CMAKE_${i}_LINKER_FLAGS${j} " ${_LDFLAGS${j}} ${OVERRIDE_LDFLAGS${j}} " CACHE STRING "" FORCE)
    endforeach()
endforeach()

set(CMAKE_BUILD_TYPE_INIT "RELEASE")
