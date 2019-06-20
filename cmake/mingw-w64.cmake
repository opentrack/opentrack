# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cd build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/mingw-w64.cmake
# -sh 20140922

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "" FORCE)
endif()

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 5)
set(CMAKE_SYSROOT "/usr/i686-w64-mingw32")

set(c "")

## specify the cross compiler
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    #set(p D:/cygwin64/opt/bin/)
    #set(p "/mingw32/bin/")
    set(p "")
    set(e .exe)
    set(c "${p}")
else()
    set(p "")
    set(c "i686-w64-mingw32-")
    set(e "")
endif()

SET(CMAKE_C_COMPILER    ${c}gcc${e})
SET(CMAKE_CXX_COMPILER  ${c}g++${e})
set(CMAKE_RC_COMPILER   ${c}windres${e})
set(CMAKE_LINKER        ${c}ld${e})
set(CMAKE_AR            ${c}gcc-ar${e})
set(CMAKE_NM            ${c}gcc-nm${e})
set(CMAKE_RANLIB        ${c}gcc-ranlib${e})
set(CMAKE_OBJCOPY       ${c}objcopy${e})
set(CMAKE_OBJDUMP       ${c}objdump${e})
set(CMAKE_STRIP         ${c}strip${e})

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(fpu "-ffast-math -mfpmath=sse -mstackrealign")
set(cpu "-O3 -msse3 -mtune=skylake")
#set(lto "-fno-lto -fno-use-linker-plugin -flto-compression-level=9 -flto-partition=balanced -fno-ipa-pta -fno-lto-odr-type-merging")
set(lto "")
set(sections "-ffunction-sections -fdata-sections")

set(cc "")
#set(cc "-fdump-statistics-details -fdump-ipa-cgraph")

set(noisy-warns "")
set(suggest-final "")
set(numerics "")
set(missing-override "")
if(WARNINGS_ENABLE)
    if(WARNINGS_FINAL_SUGGESTIONS)
        set(suggest-final "-Wsuggest-final-types")
    endif()
    if(WARNINGS_NUMERIC)
        set(numerics "-Wdouble-promotion")
    endif()
    if(WARNINGS_MISSING_OVERRIDE)
        set(missing-override "-Wsuggest-override")
    endif()
    set(noisy-warns "${suggest-final} ${numerics}")
endif()

set(_CXX_WARNS "")
set(_C_WARNS "")

if(WARNINGS_ENABLE)
    set(usual-warns "-Wstrict-aliasing=3 -Wstrict-overflow=4 -Wdelete-non-virtual-dtor -Wno-odr -Wattributes")
    set(_C_WARNS "-Wall -Wextra -Wpedantic -Wcast-align")
    set(_CXX_WARNS "${_C_WARNS} ${usual-warns} ${noisy-warns} ${missing-override}")
endif()

set(ccflags-common "-pipe -g3")
set(_CXXFLAGS "${ccflags-common} ${_CXX_WARNS}")
set(_CFLAGS "${ccflags-common} -std=c11 ${_C_WARNS}")
set(_CFLAGS_RELEASE "${cpu} ${fpu} ${lto} ${sections} ${cc}")
set(_CFLAGS_DEBUG "-g -O0 -fstack-protector-strong")
set(_CXXFLAGS_RELEASE "${_CFLAGS_RELEASE} ${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

add_definitions(-DSTRSAFE_NO_DEPRECATE)

#set(_LDFLAGS "-Wl,--dynamicbase,--nxcompat,--as-needed -Wl,--gc-sections,--exclude-libs,ALL")
set(_LDFLAGS_RELEASE "")
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
        set(CMAKE_${j}_FLAGS${i} "${CMAKE_${j}_FLAGS${j}}")
    endforeach()
endforeach()

#foreach(j "" _DEBUG _RELEASE)
#    foreach(i MODULE EXE SHARED)
#        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${CMAKE_${i}_LINKER_FLAGS${j}}")
#    endforeach()
#endforeach()
