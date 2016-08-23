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
set(cc "/Ox /arch:SSE2 /EHscr /fp:fast /GS- /GF /GL /GR- /Gy /MD /Y- /Zi /MP /W1")

set(silly "/bigobj -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_ITERATOR_DEBUG_LEVEL=0 -D_HAS_ITERATOR_DEBUGGING=0 -D_SECURE_SCL=0")

set(_CFLAGS "${silly}")
set(_CXXFLAGS "${silly}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "/Zi /GS")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "/Zi /GS")

set(ldflags-shared-release "/OPT:REF,ICF=10")
set(ldflags-shared "/DEBUG /DYNAMICBASE /NXCOMPAT")

foreach (i MODULE EXE SHARED)
    set(_LDFLAGS_${i} "${ldflags-shared}")
    set(_LDFLAGS_${i}_RELEASE "${ldflags-shared-release}")
endforeach()

set(_LDFLAGS "")
set(_LDFLAGS_RELEASE "/LTCG")
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
        set(CMAKE_${i}_LINKER_FLAGS${j} "${_LDFLAGS${j}} ${_LDFLAGS_${i}} ${_LDFLAGS_${i}_${j}} ${OVERRIDE_LDFLAGS${j}}" CACHE STRING "" FORCE)
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
        message("cmd /k call \"%vs150comntools%\"\\..\\..\\vc\\bin\\vcvars32.bat & cd /d \"${CMAKE_BINARY_DIR}\"")
        message("\n--")
        message(FATAL_ERROR "cannot continue.")
    endif()
endif()
