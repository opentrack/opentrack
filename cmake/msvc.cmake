# this file only serves as toolchain file when specified so explicitly
# when building the software. from repository's root directory:
# mkdir build && cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/../cmake/msvc.cmake build/

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)
endif()

#set(CMAKE_GENERATOR Ninja)
#set(CMAKE_MAKE_PROGRAM ninja.exe)
#set(CMAKE_ASM_NASM_COMPILER nasm.exe)

# search for programs in the host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# don't poison with system compile-time data
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


#add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
#add_compile_options(-Qvec-report:2)
#add_compile_options(-d2cgsummary)
add_definitions(-D_HAS_EXCEPTIONS=0)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    # ignore cmake warning: Manually-specified variable not used by the project
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW CACHE INTERNAL "" FORCE)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_CXX_EXTENSIONS FALSE)

function(sets type)
    list(LENGTH ARGN len)
    math(EXPR rem "${len} % 2")
    if(rem)
        message(FATAL_ERROR "argument count not even")
    endif()
    while(NOT ARGN STREQUAL "")
        list(POP_FRONT ARGN name value)
        set(${name} "${value}" CACHE "${type}" "" FORCE)
    endwhile()
endfunction()

if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    #include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)

    add_compile_options("-W4")

    # C4265: class has virtual functions, but destructor is not virtual
    # C4005: macro redefinition
    set(warns-error 4265 4005)

    foreach(i ${warns-error})
        add_compile_options(-w1${i} -we${i})
    endforeach()
endif()

if(CMAKE_PROJECT_NAME MATCHES "^Qt(Base)?$")
    set(QT_BUILD_TOOLS_WHEN_CROSSCOMPILING TRUE CACHE BOOL "" FORCE)
    set(QT_DEBUG_OPTIMIZATION_FLAGS TRUE CACHE BOOL "" FORCE)
    set(QT_USE_DEFAULT_CMAKE_OPTIMIZATION_FLAGS TRUE CACHE BOOL "" FORCE)

    set(FEATURE_debug OFF)
    set(FEATURE_debug_and_release OFF)
    set(FEATURE_force_debug_info ON)
    #set(FEATURE_ltcg ON)
    set(FEATURE_shared ON)
    add_definitions(-DNDEBUG)
endif()

if(CMAKE_PROJECT_NAME STREQUAL "OpenCV")
    set(PARALLEL_ENABLE_PLUGINS FALSE)
    set(HIGHGUI_ENABLE_PLUGINS FALSE)
    set(VIDEOIO_ENABLE_PLUGINS FALSE)

    set(OPENCV_SKIP_MSVC_EXCEPTIONS_FLAG TRUE)
    set(OPENCV_DISABLE_THREAD_SUPPORT TRUE)

    set(ENABLE_FAST_MATH    ON)
    set(BUILD_TESTS         OFF)
    set(BUILD_PERF_TESTS    OFF)
    set(BUILD_opencv_apps   OFF)
    set(BUILD_opencv_gapi   OFF)

    set(OPENCV_SKIP_MSVC_PARALLEL 1)
    set(OPENCV_DISABLE_THREAD_SUPPORT 1)
endif()

if(CMAKE_PROJECT_NAME STREQUAL "TestOscpack")
    add_compile_definitions(OSC_HOST_LITTLE_ENDIAN)
endif()

set(opentrack-simd "SSE2")

if(CMAKE_PROJECT_NAME STREQUAL "onnxruntime")
    sets(BOOL
        #ONNX_USE_MSVC_STATIC_RUNTIME           ON
        #protobuf_MSVC_STATIC_RUNTIME           ON
        #ABSL_MSVC_STATIC_RUNTIME               ON
         BUILD_SHARED_LIBS                      OFF
         BUILD_TESTING                          OFF
         onnxruntime_BUILD_BENCHMARKS           OFF
         onnxruntime_BUILD_FOR_NATIVE_MACHINE   OFF
         onnxruntime_BUILD_SHARED_LIB           ON
         onnxruntime_BUILD_UNIT_TESTS           OFF
         protobuf_BUILD_EXAMPLES                OFF
         protobuf_BUILD_SHARED_LIBS             OFF
         ONNX_BUILD_BENCHMARKS                  OFF
         ONNX_BUILD_TESTS                       OFF
         ONNX_DISABLE_EXCEPTIONS                OFF
         ONNX_GEN_PB_TYPE_STUBS                 OFF
         onnxruntime_DISABLE_CONTRIB_OPS        ON

    )
    if(opentrack-64bit)
        sets(BOOL
             onnxruntime_USE_AVX                ON
        )
    endif()
endif()

#if(opentrack-64bit)
#    set(opentrack-simd "AVX")
#endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_compile_options(-MD)

add_link_options(-cgthreads:1)

set(_CFLAGS "-diagnostics:caret -Zi -Zf -Zo -bigobj -vd0")
#if(NOT opentrack-no-static-crt)
#    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded" CACHE INTERNAL "" FORCE)
#else()
#    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE INTERNAL "" FORCE)
#endif()
set(_CXXFLAGS "${_CFLAGS}")
set(_CFLAGS_RELEASE "-O2 -Oit -Oy- -Ob3 -fp:fast -GS- -GF -Gy -Gw-")
if(NOT "${opentrack-simd}" STREQUAL "")
    set(_CFLAGS_RELEASE "${_CFLAGS_RELEASE} -arch:${opentrack-simd}")
endif()
set(_CFLAGS_DEBUG "-guard:cf -MTd -Gs0 -RTCs")
set(_CXXFLAGS_RELEASE "${_CFLAGS_RELEASE}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS "")
set(_LDFLAGS_RELEASE "-OPT:REF,ICF=10 -DEBUG:FULL")
set(_LDFLAGS_DEBUG "-DEBUG:FULL")

set(_LDFLAGS_STATIC "")
set(_LDFLAGS_STATIC_RELEASE "")
set(_LDFLAGS_STATIC_DEBUG "")

set(CMAKE_BUILD_TYPE_INIT "RELEASE" CACHE INTERNAL "")
set(CMAKE_BUILD_TYPE "RELEASE" CACHE INTERNAL "")

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
if (NOT CMAKE_BUILD_TYPE STREQUAL "RELEASE" AND NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS RELEASE DEBUG)

foreach(k "" "_${CMAKE_BUILD_TYPE}")
    set("FLAGS_CXX${k}"     "" CACHE STRING "More CMAKE_CXX_FLAGS${k}")
    set("FLAGS_C${k}"       "" CACHE STRING "More CMAKE_C_FLAGS${k} (almost never used)")
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
