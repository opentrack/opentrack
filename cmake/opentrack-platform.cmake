# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

include_guard(GLOBAL)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
    endif()
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" __build_type)
set(CMAKE_BUILD_TYPE "${__build_type}" CACHE STRING "" FORCE)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*|i[0-9]86.*|x86.*")
    set(opentrack-intel TRUE)
else()
    set(opentrack-intel FALSE)
endif()

if(CMAKE_SIZEOF_VOID_P GREATER_EQUAL 8)
    set(opentrack-64bit TRUE)
else()
    set(opentrack-64bit FALSE)
endif()

if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES  "x86_64")
    endif()
endif()

if(MSVC AND MSVC_VERSION LESS "1915" AND NOT ".${CMAKE_CXX_COMPILER_ID}" STREQUAL ".Clang")
    message(FATAL_ERROR "Visual Studio too old. Use Visual Studio 2017 or newer.")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
    set(CMAKE_COMPILER_IS_CLANGXX TRUE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if((NOT CMAKE_COMPILER_IS_GNUCXX) EQUAL (NOT (NOT CMAKE_COMPILER_IS_GNUCC)))
    # this build system has logic errors otherwise
    message(FATAL_ERROR "use either use both gcc and g++ or neither")
endif()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES=1)
endif()

if(MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_NO_DEBUG_HEAP)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    add_definitions(-D_HAS_EXCEPTIONS=0)
    add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
    add_definitions(-D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING)
    add_definitions(-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    set(__stuff "-permissive- -diagnostics:caret")
    set(CMAKE_CXX_FLAGS "${__stuff} ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${__stuff} ${CMAKE_CXX_FLAGS}")

    if(opentrack-64bit)
        set(ent "-HIGHENTROPYVA")
    else()
        set(ent "")
    endif()

    foreach (i SHARED MODULE EXE)
        # 4020 is compiler bug for opentrack-cv
        set(CMAKE_${i}_LINKER_FLAGS "-DYNAMICBASE -NXCOMPAT -DEBUG -ignore:4020 ${ent} ${CMAKE_${i}_LINKER_FLAGS}")
    endforeach()
endif()

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> --use-temp-file -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  enable_language(RC)
endif()

if(opentrack-install-rpath)
    set(CMAKE_INSTALL_RPATH "${opentrack-install-rpath}")
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
endif()

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories("${CMAKE_SOURCE_DIR}")

if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    set(_common "-fvisibility=hidden")
    set(CMAKE_C_FLAGS "${_common} ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${_common} -fuse-cxa-atexit ${CMAKE_CXX_FLAGS}")
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
    set(apple-frameworks "-stdlib=libc++ -framework Cocoa -framework CoreFoundation -lobjc -lz -framework Carbon")
    set(CMAKE_SHARED_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_SHARED_LINKER_FLAGS}")
    #set(CMAKE_STATIC_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_DEFAULT 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

foreach(k _RELEASE _DEBUG _RELWITHDEBINFO _MINSIZEREL)
    set(CMAKE_C_FLAGS${k} "${CMAKE_C_FLAGS${k}} -UNDEBUG")
    set(CMAKE_CXX_FLAGS${k} "${CMAKE_CXX_FLAGS${k}} -UNDEBUG")
endforeach()

if(MINGW)
    add_definitions(-DMINGW_HAS_SECURE_API)
    add_definitions(-DSTRSAFE_NO_DEPRECATE)
endif()

# assume binutils
if(UNIX AND NOT APPLE)
    foreach (i SHARED MODULE EXE)
        set(CMAKE_${i}_LINKER_FLAGS "-Wl,-z,relro,-z,now,--exclude-libs,ALL ${CMAKE_${i}_LINKER_FLAGS}")
    endforeach()
    include(opentrack-pkg-config)
endif()

set(opentrack_maintainer-mode FALSE CACHE BOOL "Select if developing core code (not modules)")
