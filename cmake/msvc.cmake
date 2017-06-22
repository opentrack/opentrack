# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/msvc.cmake build/

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# oldest CPU supported here is Northwood-based Pentium 4. -sh 20150811
set(cc "/O2 /O2it /Ob2 /fp:fast /GS- /GF /GL /Gw /Gy /Gm /Zc:inline /Zo /FS /Zc:threadSafeInit /arch:SSE2 -D_HAS_EXCEPTIONS=0")

set(warns_ "")

set(warns-disable 4530 4577 4789 4244 4702 4530 4244)

foreach(i ${warns-disable})
    set(warns_ "${warns_} /wd${i}")
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
        set(warns_ "${warns_} /w1${i} /we${i}")
    endforeach()

    foreach(i ${warns-noerr})
        set(warns_ "${warns_} /w1${i}")
    endforeach()
    set(cc "${cc} /GR-")
endif()

set(base-cflags "${warns_} -MT -Zi -cgthreads8")

set(_CFLAGS "${base-cflags}")
set(_CXXFLAGS "${base-cflags}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "/GS /sdl /Gs /guard:cf")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "/machine:X86 /DEBUG")
set(_LDFLAGS_RELEASE "/LTCG:INCREMENTAL /OPT:REF /OPT:ICF=10")
set(_LDFLAGS_DEBUG "")

set(_LDFLAGS_STATIC "/machine:X86 /WX")
set(_LDFLAGS_STATIC_RELEASE "/LTCG:INCREMENTAL")
set(_LDFLAGS_STATIC_DEBUG "")

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        set(CMAKE_${j}_FLAGS${i} "${CMAKE_${j}_FLAGS${i}} ${_${j}FLAGS${i}}")
    endforeach()
    set(CMAKE_${j}_FLAGS "${CMAKE_${j}_FLAGS} ${_${j}FLAGS}")
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${CMAKE_${i}_LINKER_FLAGS${j}}")
    endforeach()
endforeach()

set(CMAKE_STATIC_LINKER_FLAGS "${_LDFLAGS_STATIC} ${CMAKE_STATIC_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${_LDFLAGS_STATIC_RELEASE} ${CMAKE_STATIC_LINKER_FLAGS_RELEASE}")
set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${_LDFLAGS_STATIC_DEBUG} ${CMAKE_STATIC_LINKER_FLAGS_DEBUG}")

set(CMAKE_BUILD_TYPE_INIT RELEASE)
# for nmake/jom build directories
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

set(CMAKE_RC_FLAGS "-nologo -DWIN32")

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake")

