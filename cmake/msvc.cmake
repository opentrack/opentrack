# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/msvc.cmake build/

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 4.0)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(cc "")
# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(cc "${cc} -O2 -O2it -Oy- -Ob2 -fp:fast -GS- -GF -GL -Gw -Gy -Gs9999999")
set(cc "${cc} -FS -arch:SSE2 -D_HAS_EXCEPTIONS=0")
#set(cc "${cc} -Qvec-report:1")

set(warns_ "")
set(warns-disable 4530 4577 4789 4244 4702 4530 4244 4127 4458 4456 4251 4100 4702 4457)

if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)
    #C4457: declaration of 'id' hides function parameter
    #C4456: declaration of 'i' hides previous local declaration
    #C4263 - member function does not override any base class virtual member function
    #C4264 - no override available for virtual member function from base class, function is hidden
    #C4265 - class has virtual functions, but destructor is not virtual
    #C4266 - no override available for virtual member function from base type, function is hidden
    #C4928 - illegal copy-initialization, more than one user-defined conversion has been implicitly applied

    foreach(k CMP0020 CMP0022 CMP0058 CMP0028 CMP0042 CMP0063 CMP0053 CMP0011 CMP0054 CMP0012)
        if(POLICY ${k})
            cmake_policy(SET ${k} NEW)
        endif()
    endforeach()

    # C4265: class has virtual functions, but destructor is not virtual
    set(warns 4265)
    # C4005: macro redefinition
    set(warns-noerr 4005)

    foreach(i ${warns-disable})
        set(warns_ "${warns_} -wd${i}")
    endforeach()

    foreach(i ${warns})
        set(warns_ "${warns_} -w1${i} -we${i}")
    endforeach()

    foreach(i ${warns-noerr})
        set(warns_ "${warns_} -w1${i}")
    endforeach()

    set(cc "${cc} -GR-")

    set(CMAKE_RC_FLAGS "/nologo /DWIN32")
endif()

set(base-cflags "-MT -Zi -Zf -Zo -W4 -Zo -bigobj -cgthreads1 -diagnostics:caret ${warns_}")
#set(base-cflags "${base-cflags} -d2cgsummary")

set(_CFLAGS "${base-cflags}")
set(_CXXFLAGS "${base-cflags}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "-GS -sdl -Gs -guard:cf")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "-cgthreads:1 -DEBUG")

#set(_ltcg "-LTCG")
set(_ltcg "-LTCG:INCREMENTAL")

set(_LDFLAGS_RELEASE "-OPT:REF,ICF=4 ${_ltcg}")
set(_LDFLAGS_DEBUG "")

set(_LDFLAGS_STATIC "")
set(_LDFLAGS_STATIC_RELEASE "${_ltcg}")
set(_LDFLAGS_STATIC_DEBUG "")

# debugging build times
#set(_CXXFLAGS "${_CXXFLAGS} -Bt+")
#set(_LDFLAGS "${_LDFLAGS} -time")

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" __build_type)
if(NOT __build_type STREQUAL CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "${__build_type}" CACHE STRING "" FORCE)
endif()

if((NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG") AND (NOT CMAKE_BUILD_TYPE STREQUAL "RELEASE"))
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS RELEASE DEBUG)

foreach(k "" "_${CMAKE_BUILD_TYPE}")
    set("FLAGS_CXX${k}"     "" CACHE STRING "More CMAKE_CXX_FLAGS${k}")
    #set("FLAGS_C${k}"     "" CACHE STRING "More CMAKE_C_FLAGS${k} (almost never used)")
    set("FLAGS_LD${k}"      "" CACHE STRING "More CMAKE_(SHARED|EXE|MODULE)_LINKER_FLAGS${k}")
    set("FLAGS_ARCHIVE${k}" "" CACHE STRING "More CMAKE_STATIC_LINKER_FLAGS${k}")
endforeach()

foreach(k "" _DEBUG _RELEASE)
    #set(CMAKE_STATIC_LINKER_FLAGS${k} "${CMAKE_STATIC_LINKER_FLAGS${k}} ${_LDFLAGS_STATIC${k}}")
    set(CMAKE_STATIC_LINKER_FLAGS${k} "${_LDFLAGS_STATIC${k}} ${FLAGS_ARCHIVE${k}}" CACHE STRING "" FORCE)
endforeach()
foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        #set(CMAKE_${i}_LINKER_FLAGS${j} "${CMAKE_${i}_LINKER_FLAGS${j}} ${_LDFLAGS${j}}")
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${FLAGS_LD${j}}" CACHE STRING "" FORCE)
    endforeach()
endforeach()

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        #set(CMAKE_${j}_FLAGS${i} "${CMAKE_${j}_FLAGS${i}} ${_${j}FLAGS${i}}")
        set(CMAKE_${j}_FLAGS${i} "${_${j}FLAGS${i}} ${FLAGS_${j}${i}}" CACHE STRING "" FORCE)
    endforeach()
endforeach()
