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
set(cc "/O2it /Ob2 /arch:SSE2 /fp:fast /GS- /GF /GL /Gw /Gy")

set(warns_ "")

set(warns-disable 4530 4577 4789)

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

set(silly "${warns_} /MT /Zi /Gm")

set(_CFLAGS "${silly}")
set(_CXXFLAGS "${silly}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "/GS /sdl /Gs /guard:cf -D_ITERATOR_DEBUG_LEVEL=0 -D_HAS_ITERATOR_DEBUGGING=0 -D_SECURE_SCL=0")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(ldflags-shared-release "/OPT:REF /OPT:ICF=10")
set(ldflags-shared "/DEBUG")

foreach (i MODULE EXE SHARED)
    set(_LDFLAGS_${i} "${ldflags-shared}")
    set(_LDFLAGS_${i}_RELEASE "${ldflags-shared-release}")
endforeach()

set(_LDFLAGS "")
set(_LDFLAGS_RELEASE "/LTCG:INCREMENTAL")
set(_LDFLAGS_DEBUG "")

foreach(j C CXX)
    foreach(i "" _DEBUG _RELEASE)
        set(OVERRIDE_${j}_FLAGS${i} "" CACHE STRING "")
        set(CMAKE_${j}_FLAGS${i} "${_${j}FLAGS${i}} ${OVERRIDE_${j}_FLAGS${i}}" CACHE STRING "" FORCE)
    endforeach()
    set(CMAKE_${j}_FLAGS "${_${j}FLAGS} ${_${j}_WARNS} ${OVERRIDE_${j}_FLAGS}" CACHE STRING "" FORCE)
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED STATIC)
        set(OVERRIDE_LDFLAGS${j} "" CACHE STRING "")
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${_LDFLAGS_${i}} ${_LDFLAGS_${i}${j}} ${OVERRIDE_LDFLAGS${j}}" CACHE STRING "" FORCE)
    endforeach()
endforeach()

foreach(j "" _DEBUG _RELEASE)
    foreach(i MODULE EXE SHARED)
        set(OVERRIDE_LDFLAGS_SHARED${j} "" CACHE STRING "")
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${_LDFLAGS_${i}} ${_LDFLAGS_${i}${j}} ${OVERRIDE_LDFLAGS_SHARED${j}}" CACHE STRING "" FORCE)
    endforeach()
endforeach()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "" FORCE)

set(CMAKE_BUILD_TYPE_INIT RELEASE)

# for nmake/jom build directories
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
endif()

if((CMAKE_GENERATOR STREQUAL "NMake Makefiles") OR (CMAKE_GENERATOR STREQUAL "NMake Makefiles JOM"))
    if("$ENV{LIBPATH}" STREQUAL "")
        message("Error, no environment. Run:")
        message("--\n")
        message("cmd /k call \"%vs140comntools%\"\\..\\..\\vc\\bin\\vcvars32.bat & cd /d \"${CMAKE_BINARY_DIR}\"")
        message("\n--")
        message(FATAL_ERROR "cannot continue.")
    endif()
    if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
        set(warn-flag-found FALSE)
        foreach (i CMAKE_CXX_FLAGS CMAKE_C_FLAGS)
            string(REGEX MATCH "((^| )/[W][0-9]( |\$))" ret "${${i}}")
            if(ret STREQUAL "")
                set(${i} "-W3 ${${i}}" CACHE STRING "" FORCE)
            endif()
        endforeach()
    endif()
endif()

set(CMAKE_RC_FLAGS "-nologo -DWIN32" CACHE STRING "" FORCE)
