# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 5)

# specify the cross compiler
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    #set(p D:/cygwin64/opt/bin/)
    set(p "C:/msys64/mingw32/bin/")
    set(e .exe)
    set(c "${p}")
else()
    set(p "")
    set(c "${p}i686-w64-mingw32-")
    set(e "")
endif()

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


# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(fpu "-ffast-math -mfpmath=both -mstackrealign -falign-functions=16 -falign-loops=16")
set(cpu "-O3 -march=i686 -msse3 -mtune=skylake -frename-registers")
#set(lto "-fno-lto -fno-use-linker-plugin -flto-compression-level=9 -flto-partition=balanced -fno-ipa-pta -fno-lto-odr-type-merging")
set(lto "")
set(sections "-ffunction-sections -fdata-sections -s")

set(cc "")
#set(cc "-fdump-statistics-details -fdump-ipa-cgraph")

set(clang-warns "")
if(CMAKE_COMPILER_IS_CLANG)
    set(clang-warns "-Wweak-vtables")
endif()

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

set(_CXX_WARNS "")
set(_C_WARNS "")

if(WARNINGS_ENABLE)
    set(usual-warns "-Wdelete-non-virtual-dtor -Wno-suggest-override -Wno-odr -Wno-attributes -Wcast-align")
    set(_C_WARNS "-Wall -Wextra -Wpedantic -Wcast-align")
    set(_CXX_WARNS "${_C_WARNS} ${usual-warns} ${clang-warns} ${noisy-warns} ${missing-override}")
endif()

set(ccflags-common "-fvisibility=hidden -pipe -g3")
set(_CXXFLAGS "${ccflags-common} ${_CXX_WARNS}")
set(_CFLAGS "${ccflags-common} -std=c11 ${_C_WARNS}")
set(_CFLAGS_RELEASE "${cpu} ${fpu} ${lto} ${sections} ${cc}")
set(_CFLAGS_DEBUG "-g -O0 -fstack-protector-strong")
set(_CXXFLAGS_RELEASE "${_CFLAGS_RELEASE} ${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "-Wl,--dynamicbase,--nxcompat,--as-needed")
set(_LDFLAGS_RELEASE "-Wl,--gc-sections,--exclude-libs,ALL")
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

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        set(CMAKE_${j}_FLAGS${i} "${_${j}FLAGS${i}} ${CMAKE_${j}_FLAGS${j}}")
    endforeach()
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${CMAKE_${i}_LINKER_FLAGS${j}}")
    endforeach()
endforeach()

if(NOT __otr_toolchain_initialized)
    set(__otr_toolchain_initialized 1 CACHE INTERNAL "" FORCE)
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
endif()
