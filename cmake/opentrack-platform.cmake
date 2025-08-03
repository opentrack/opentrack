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

if(MSVC AND MSVC_VERSION LESS "1915" AND NOT ".${CMAKE_CXX_COMPILER_ID}" STREQUAL ".Clang")
    message(FATAL_ERROR "Visual Studio too old. Use Visual Studio 2017 or newer.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)

include_directories("${CMAKE_SOURCE_DIR}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_DEFAULT 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)


if(NOT WIN32 AND NOT APPLE)
    include(opentrack-pkg-config)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCXX TRUE)
    set(CMAKE_COMPILER_IS_CLANGXX TRUE)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_COMPILER_IS_GNUCC TRUE)
    set(CMAKE_COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*|i[0-9]86.*|x86.*")
    set(opentrack-intel TRUE)
elseif(MSVC AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "")
    set(opentrack-intel TRUE)
else()
    set(opentrack-intel FALSE)
endif()

if(CMAKE_SIZEOF_VOID_P GREATER_EQUAL 8)
    set(opentrack-64bit TRUE)
else()
    set(opentrack-64bit FALSE)
endif()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
    add_compile_options(-fuse-cxa-atexit)

    if(LINUX) # assume binutils
        add_link_options(-Wl,--exclude-libs,ALL)
        add_link_options(-Wl,-z,relro,-z,now)
        add_link_options(-Wl,--as-needed)
        add_link_options(-Wl,-z,noexecstack)
        add_compile_options(-fno-plt)
    endif()
endif()

if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES=1 -DSTRSAFE_NO_DEPRECATE)
endif()

if(MINGW)
    add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(MSVC)
    set(CMAKE_RC_FLAGS "/nologo /DWIN32")

    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)

    #add_compile_options(-EHsc)
    add_definitions(-D_HAS_EXCEPTIONS=0)

    add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
    add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)
    add_definitions(-D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING)
    add_definitions(-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

    add_compile_options(-permissive-)

    if(opentrack-64bit)
        add_link_options(-HIGHENTROPYVA)
    endif()

    add_link_options(-DYNAMICBASE -NXCOMPAT)
    #add_link_options(-WX)
    add_link_options(-ignore:4020)
    add_link_options(-ignore:4217) # debug build


    if(MSVC_VERSION GREATER_EQUAL 1913)
        if(NOT MSVC_VERSION GREATER_EQUAL 1929)
            add_compile_options(-experimental:external)
        endif()
        add_compile_options(-external:W0 -external:anglebrackets)
    endif()
    add_compile_options(-Zc:preprocessor)
    #add_compile_options(-Zc:inline)

    #C4457: declaration of 'id' hides function parameter
    #C4456: declaration of 'i' hides previous local declaration
    #C4263 - member function does not override any base class virtual member function
    #C4264 - no override available for virtual member function from base class, function is hidden
    #C4265 - class has virtual functions, but destructor is not virtual
    #C4266 - no override available for virtual member function from base type, function is hidden
    #C4928 - illegal copy-initialization, more than one user-defined conversion has been implicitly applied
    #C4200: nonstandard extension used: zero-sized array in struct/union
    #C4459: declaration of 'eps' hides global declaration

    set(warns-disable 4530 4577 4789 4244 4702 4530 4244 4127 4458 4456 4251 4100 4702 4457 4200 4459)

    foreach(i ${warns-disable})
        add_compile_options(-wd${i})
    endforeach()
endif()

if(NOT MSVC)
    include(FindPkgConfig)
endif()

if(LINUX AND CMAKE_COMPILER_IS_CLANG)
    link_libraries(atomic)
endif()
