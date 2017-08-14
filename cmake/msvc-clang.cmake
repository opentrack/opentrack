# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/msvc.cmake build/

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake")

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(cc "")

set(cc "${cc} -W3 -Wall -Wextra -Wno-unused-command-line-argument -Wno-missing-braces")
set(cc "${cc} -Wno-inconsistent-missing-override")
set(cc "${cc} -Wno-return-type-c-linkage")

set(cc "${cc} -Xclang -std=c++14 -Xclang -fms-compatibility-version=1912 -fms-compatibility")

set(cc "${cc} -U __clang__ -U__clang")

set(cc "${cc} -O3")
set(cc "${cc} -Xclang -O3 -Xclang -flto -Qvec -Oit -Oy -Ob2 -fp:fast -GS- -GF -GL -Gw -Gy -Gm -Zc:inline")
set(cc "${cc} -Zo -FS -Zc:threadSafeInit -arch:SSE2 -D_HAS_EXCEPTIONS=0")
set(cc "${cc} -bigobj")
#set(cc "${cc} -Wno-unknown-argument -Wno-unknown-pragmas -Wno-invalid-noreturn")

#set(CMAKE_CXX_SIMULATE_ID "MSVC" CACHE INTERNAL "" FORCE)
#set(CMAKE_C_SIMULATE_ID "MSVC" CACHE INTERNAL "" FORCE)
#set(CMAKE_CXX_SIMULATE_VERSION 19.0 CACHE INTERNAL "" FORCE)
#set(CMAKE_C_SIMULATE_VERSION 19.0 CACHE INTERNAL "" FORCE)

#set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-Xclang -isystem")
#set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-Xclang -isystem")

#set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES TRUE)
#set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES TRUE)

set(CMAKE_CXX_COMPILER "d:/llvm/msbuild-bin/cl.exe")
set(CMAKE_C_COMPILER "d:/llvm/msbuild-bin/cl.exe")

#set(CMAKE_CXX_COMPILER_ID "MSVC" CACHE INTERNAL "" FORCE)
#set(CMAKE_C_COMPILER_ID "MSVC" CACHE INTERNAL "" FORCE)

#set(CMAKE_CXX_COMPILER_VERSION "19.0" CACHE INTERNAL "" FORCE)
#set(CMAKE_C_COMPILER_VERSION "19.0" CACHE INTERNAL "" FORCE)

#set(CMAKE_VS_PLATFORM_TOOLSET "v150_clang_4_0")

set(CMAKE_CXX_FLAGS_RELEASE_INIT " ")
set(CMAKE_C_FLAGS_RELEASE_INIT " ")

set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(warns_ "")

set(warns-disable 4530 4577 4789 4244 4702 4530 4244 4127 4458 4456 4251)

foreach(i ${warns-disable})
    set(warns_ "${warns_} -wd${i}")
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

set(_CFLAGS "${base-cflags}")
set(_CXXFLAGS "${base-cflags}")
set(_CFLAGS_RELEASE "${cc}")
set(_CFLAGS_DEBUG "-GS -sdl -Gs -guard:cf")
set(_CXXFLAGS_RELEASE "${cc}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS_COMMON "-ignore:4217 -ignore:4221")
set(_LDFLAGS "-machine:X86 -DEBUG ${_LDFLAGS_COMMON}")
set(_LDFLAGS_RELEASE "-OPT:REF -OPT:ICF=10")
set(_LDFLAGS_DEBUG "")

set(_LDFLAGS_STATIC "-machine:X86 ${_LDFLAGS_COMMON}")
set(_LDFLAGS_STATIC_RELEASE "")
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

set(CMAKE_RC_FLAGS "-nologo -DWIN32")

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
endif()

