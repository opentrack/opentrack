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
set(cc "/O2it /Ob2 /fp:fast /GS- /GF /GL /Gw /Gy")

set(warns_ "")

#C4244: 'return': conversion from '__int64' to 'long', possible loss of data
set(warns-disable 4530 4577 4789 4244)

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
    set(cc "${cc} /GR- /arch:SSE2")
endif()

set(silly "${warns_} /MT /Gm")

set(_CFLAGS "${silly}")
set(_CXXFLAGS "${silly}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "/GS /sdl /Gs /guard:cf")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "/WX")
set(_LDFLAGS_RELEASE "/LTCG:INCREMENTAL /OPT:REF /OPT:ICF=10")
set(_LDFLAGS_DEBUG "")

set(_LDFLAGS_STATIC "/WX")
set(_LDFLAGS_STATIC_RELEASE "/LTCG:INCREMENTAL")
set(_LDFLAGS_STATIC_DEBUG "")

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

set(OVERRIDE_LDFLAGS_STATIC "" CACHE STRING "")
set(OVERRIDE_LDFLAGS_STATIC_RELEASE "" CACHE STRING "")
set(OVERRIDE_LDFLAGS_STATIC_DEBUG "" CACHE STRING "")

set(CMAKE_STATIC_LINKER_FLAGS "${_LDFLAGS_STATIC} ${OVERRIDE_LDFLAGS_STATIC}" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${_LDFLAGS_STATIC_RELEASE} ${OVERRIDE_LDFLAGS_STATIC_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${_LDFLAGS_STATIC_DEBUG} ${OVERRIDE_LDFLAGS_STATIC_DEBUG}" CACHE STRING "" FORCE)

set(CMAKE_BUILD_TYPE_INIT RELEASE)
# for nmake/jom build directories
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
endif()

if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    foreach (i CMAKE_CXX_FLAGS CMAKE_C_FLAGS)
        string(REGEX MATCH "((^| )[-/][W][0-9]( |\$))" ret "${${i}}")
        if(ret STREQUAL "")
            set(${i} "-W3 ${${i}}" CACHE STRING "" FORCE)
        endif()
    endforeach()
endif()

set(CMAKE_RC_FLAGS "-nologo -DWIN32" CACHE STRING "" FORCE)
