# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/msvc.cmake build/

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 6.0)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(cc "")
# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(cc "${cc} -O2 -O2it -Oy- -Ob2 -fp:fast -GS- -GF -GL -Gw -Gy -Gm")
set(cc "${cc} -Zo -FS -Zc:threadSafeInit -arch:SSE2 -D_HAS_EXCEPTIONS=0")
set(cc "${cc} -bigobj")
set(cc "${cc} -Zc:inline -Zc:rvalueCast -Zc:sizedDealloc -Zc:throwingNew")
set(cc "${cc} -Qvec-report:1")

set(warns_ "")

set(warns-disable 4530 4577 4789 4244 4702 4530 4244 4127 4458 4456 4251 4100)

foreach(i ${warns-disable})
    set(warns_ "${warns_} -wd${i}")
endforeach()

foreach(k CMP0020 CMP0022 CMP0058 CMP0028 CMP0042 CMP0063 CMP0053 CMP0011 CMP0054 CMP0012)
    if(POLICY ${k})
        cmake_policy(SET ${k} NEW)
    endif()
endforeach()

if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    #C4263 - member function does not override any base class virtual member function
    #C4264 - no override available for virtual member function from base class, function is hidden
    #C4265 - class has virtual functions, but destructor is not virtual
    #C4266 - no override available for virtual member function from base type, function is hidden
    #C4928 - illegal copy-initialization, more than one user-defined conversion has been implicitly applied

    set(warns 4263 4264 4266 4928)
    set(warns-noerr 4265)

    foreach(i ${warns})
        set(warns_ "${warns_} -w1${i} -we${i}")
    endforeach()

    foreach(i ${warns-noerr})
        set(warns_ "${warns_} -w1${i}")
    endforeach()
    set(cc "${cc} -GR-")
endif()

set(base-cflags "${warns_} -MT -Zi -cgthreads8 -W4")
#set(base-cflags "${base-cflags} -d2cgsummary")
#set(base-cflags "${base-cflags} -Bt")

set(_CFLAGS "${base-cflags}")
set(_CXXFLAGS "${base-cflags}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "-GS -sdl -Gs -guard:cf")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "-machine:X86 -DEBUG")
set(_LDFLAGS_RELEASE "-LTCG:INCREMENTAL -OPT:REF -OPT:ICF=10")
set(_LDFLAGS_DEBUG "")

set(_LDFLAGS_STATIC "-machine:X86 -WX")
set(_LDFLAGS_STATIC_RELEASE "-LTCG:INCREMENTAL")
set(_LDFLAGS_STATIC_DEBUG "")

set(CMAKE_RC_FLAGS "/nologo -DWIN32")

set(new-__otr_already_initialized "_${cc}_${base-cflags}_")
if(NOT "${__otr_already_initialized}" STREQUAL "${new-__otr_already_initialized}")
    set(__otr_already_initialized "${cc}__${base-cflags}" CACHE INTERNAL "" FORCE)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)

    set(CMAKE_STATIC_LINKER_FLAGS "${_LDFLAGS_STATIC}" CACHE STRING "" FORCE)
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${_LDFLAGS_STATIC_RELEASE}" CACHE STRING "" FORCE)
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${_LDFLAGS_STATIC_DEBUG}" CACHE STRING "" FORCE)

    foreach(j C CXX)
        foreach(i "" _DEBUG _RELEASE)
            set(CMAKE_${j}_FLAGS${i} "${_${j}FLAGS${i}}" CACHE STRING "" FORCE)
        endforeach()
    endforeach()

    foreach(j "" _DEBUG _RELEASE)
        foreach(i MODULE EXE SHARED)
            set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}}" CACHE STRING "" FORCE)
        endforeach()
    endforeach()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake")
