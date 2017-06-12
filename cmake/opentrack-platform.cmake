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

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_GNUC TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if((NOT CMAKE_COMPILER_IS_GNUCXX) EQUAL (NOT (NOT CMAKE_COMPILER_IS_GNUCC)))
    message(FATAL_ERROR "cannot use either use both gcc and g++ or neither")
endif()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(MSVC)
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0 -D_ITERATOR_DEBUG_LEVEL=0)
    add_definitions(-D_HAS_EXCEPTIONS=0)

    set(CMAKE_CXX_FLAGS "-std:c++14 ${CMAKE_CXX_FLAGS}")
    if(SDK_INSTALL_DEBUG_INFO)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Zi")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Zi")
    endif()

    foreach (i SHARED MODULE EXE)
        set(CMAKE_${i}_LINKER_FLAGS "${CMAKE_${i}_LINKER_FLAGS} -DYNAMICBASE -NXCOMPAT")
        if(SDK_INSTALL_DEBUG_INFO)
            set(CMAKE_${i}_LINKER_FLAGS "${CMAKE_${i}_LINKER_FLAGS} -DEBUG")
        endif()
    endforeach()
endif()

if(WIN32)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_RC_COMPILER_INIT i686-w64-mingw32-windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> --use-temp-file -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif()
  enable_language(RC)
  add_definitions(-D_USE_MATH_DEFINES=1)
endif(WIN32)

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

if(APPLE)
    set(CMAKE_MACOSX_RPATH OFF)
    set(apple-frameworks "-stdlib=libc++ -framework Cocoa -framework CoreFoundation -lobjc -lz -framework Carbon")
    set(CMAKE_SHARED_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_SHARED_LINKER_FLAGS}")
    #set(CMAKE_STATIC_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS " ${apple-frameworks} ${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS " -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

if(NOT MSVC)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_DEFAULT 14)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
    set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS OFF)

function(fix_flags lang flag replacement)
    set(part "-")
    set(pfx "-")
    if(MSVC)
        set(part "[-/]")
        set(pfx "/")
    endif()
    if(".${replacement}" STREQUAL ".")
        set(pfx "")
    endif()
    foreach(k _DEBUG _RELEASE _MINSIZEREL _RELWITHDEBINFO "")
        set(tmp "${CMAKE_${lang}_FLAGS${k}}")
        if(NOT ".${replacement}" STREQUAL ".")
            string(APPEND replacement " ")
        endif()
        string(REGEX REPLACE "(^| )${part}${flag}(\$| )" " ${pfx}${replacement}" CMAKE_${lang}_FLAGS${k} "${CMAKE_${lang}_FLAGS${k}}")
        if(NOT ".${tmp}" STREQUAL ".${CMAKE_${lang}_FLAGS${k}}")
            set(CMAKE_${lang}_FLAGS${k} "${CMAKE_${lang}_FLAGS${k}}" CACHE STRING "")
        else()
            set(CMAKE_${lang}_FLAGS${k} "${CMAKE_${lang}_FLAGS${k}} ${pfx}${replacement}" CACHE STRING "")
        endif()
    endforeach()
endfunction()

# nix -rdynamic passed from Linux-GNU.cmake
if(CMAKE_COMPILER_IS_GNUCXX)
    set(__LINUX_COMPILER_GNU 1)
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
endif()

if(MINGW)
    add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(UNIX AND NOT APPLE)
    include(FindPkgConfig)
endif()

set(opencv-modules opencv_calib3d opencv_core opencv_features2d opencv_imgcodecs opencv_imgproc opencv_objdetect opencv_videoio)
