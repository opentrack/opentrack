# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

unset(c)
unset(e)
# specify the cross compiler
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(p C:/msys64/mingw64/bin/)
    set(e .exe)
endif()
set(c ${p}i686-w64-mingw32-)
#set(CMAKE_MAKE_PROGRAM ${p}/mingw32-make.exe CACHE FILEPATH "" FORCE)

SET(CMAKE_C_COMPILER    ${c}gcc${e})
SET(CMAKE_CXX_COMPILER  ${c}g++${e})
set(CMAKE_RC_COMPILER   ${c}windres${e})
set(CMAKE_LINKER        ${c}ld${e})
set(CMAKE_AR            ${c}gcc-ar${e}      CACHE STRING "" FORCE)
set(CMAKE_NM            ${c}gcc-nm${e}      CACHE STRING "" FORCE)
set(CMAKE_RANLIB        ${c}gcc-ranlib${e}  CACHE STRING "" FORCE)
set(CMAKE_OBJCOPY       ${c}objcopy${e}     CACHE STRING "" FORCE)
set(CMAKE_OBJDUMP       ${c}objdump${e}     CACHE STRING "" FORCE)
set(CMAKE_STRIP         ${c}strip${e}       CACHE STRING "" FORCE)

#SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(fpu "-ffast-math -mfpmath=both -mstackrealign -falign-functions=16 -falign-loops=16")
set(cpu "-O3 -march=pentium4 -mtune=corei7-avx -msse -msse2 -mno-sse3 -mno-avx -frename-registers")
set(lto "-flto -fuse-linker-plugin -flto-compression-level=9 -flto-partition=none -fipa-icf -fipa-pta -flto-odr-type-merging")
set(bloat "-fno-exceptions -fno-rtti")
set(sections "-ffunction-sections -fdata-sections")

set(ccflags-common "-fvisibility=hidden -pipe")
set(_CXXFLAGS "${bloat} ${ccflags-common} -std=c++11")
set(_CFLAGS "${ccflags-common} -std=c11")
set(_CFLAGS_RELEASE "-s ${cpu} ${fpu} ${lto} ${sections}")
set(_CFLAGS_DEBUG "-g -O0 -fstack-protector-strong")
set(_CXXFLAGS_RELEASE "${_CFLAGS_RELEASE}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "-Wl,--dynamicbase,--no-seh,--nxcompat,--as-needed")
set(_LDFLAGS_RELEASE "-Wl,--gc-sections,--exclude-libs,ALL -ffunction-sections")
set(_LDFLAGS_DEBUG "")

set(enable-val FALSE)
if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    set(enable-val TRUE)
endif()

set(WARNINGS_ENABLE ${enable-val} CACHE BOOL "Emit additional warnings at compile-time")
# these are very noisy, high false positive rate. only for development.
set(WARNINGS_FINAL_SUGGESTIONS FALSE CACHE BOOL "Emit very noisy warnings at compile-time")
set(WARNINGS_NUMERIC ${enable-val} CACHE BOOL "Emit very noisy warnings at compile-time")
set(WARNINGS_MISSING_OVERRIDE FALSE CACHE BOOL "Emit very noisy warnings at compile-time")

set(noisy-warns "")
set(suggest-final "")
set(numerics "")
set(missing-override "")
if(WARNINGS_ENABLE)
    if(WARNINGS_FINAL_SUGGESTIONS)
        set(suggest-final "-Wsuggest-final-types -Wsuggest-final-methods")
    endif()
    if(WARNINGS_NUMERIC)
        set(numerics "-Wdouble-promotion -Wsign-compare")
    endif()
    if(WARNINGS_MISSING_OVERRIDE)
        set(missing-override "-Wsuggest-override")
    endif()
    set(noisy-warns "${suggest-final} ${numerics}")
endif()

set(clang-warns "")
if(CMAKE_COMPILER_IS_CLANG)
    set(clang-warns "-Wweak-vtables")
endif()
set(_CXX_WARNS "")
set(_C_WARNS "")
if(WARNINGS_ENABLE)
    set(usual-warns "-Wall -Wextra -pedantic -Wdelete-non-virtual-dtor -Wno-suggest-override -Wno-odr -Wno-attributes -Wcast-align -Wc++14-compat")
    set(_CXX_WARNS "${usual-warns} ${clang-warns} ${noisy-warns} ${missing-override}")
    set(_C_WARNS "-Wall -Wextra -pedantic -Wcast-align")
endif()

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        set(OVERRIDE_${j}_FLAGS${i} "" CACHE STRING "")
        set(CMAKE_${j}_FLAGS${i} "${_${j}FLAGS${i}} ${OVERRIDE_${j}_FLAGS${i}}" CACHE STRING "" FORCE)
    endforeach()
    set(CMAKE_${j}_FLAGS "${_${j}FLAGS} ${_${j}_WARNS} ${OVERRIDE_${j}_FLAGS}" CACHE STRING "" FORCE)
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        set(OVERRIDE_LDFLAGS${j} "" CACHE STRING "")
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${OVERRIDE_LDFLAGS${j}}" CACHE STRING "" FORCE)
    endforeach()
endforeach()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "" FORCE)

