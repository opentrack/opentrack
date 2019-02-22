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

#C4457: declaration of 'id' hides function parameter
#C4456: declaration of 'i' hides previous local declaration
#C4263 - member function does not override any base class virtual member function
#C4264 - no override available for virtual member function from base class, function is hidden
#C4265 - class has virtual functions, but destructor is not virtual
#C4266 - no override available for virtual member function from base type, function is hidden
#C4928 - illegal copy-initialization, more than one user-defined conversion has been implicitly applied
set(warns-disable 4530 4577 4789 4244 4702 4530 4244 4127 4458 4456 4251 4100 4702 4457)

add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)
add_definitions(-D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING)
add_definitions(-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
add_definitions(-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
add_definitions(-diagnostics:caret)
#add_compile_options(-Qvec-report:2)
#add_compile_options(-d2cgsummary)

if(NOT CMAKE_RC_FLAGS)
    set(CMAKE_RC_FLAGS "/nologo /DWIN32")
endif()

if(CMAKE_PROJECT_NAME STREQUAL "opentrack")
    include("${CMAKE_CURRENT_LIST_DIR}/opentrack-policy.cmake" NO_POLICY_SCOPE)

    # C4265: class has virtual functions, but destructor is not virtual
    set(warns 4265)
    # C4005: macro redefinition
    set(warns-noerr 4005)

    foreach(i ${warns-disable})
        add_compile_options(-wd${i})
    endforeach()

    foreach(i ${warns})
        add_compile_options(-w1${i} -we${i})
    endforeach()

    foreach(i ${warns-noerr})
        add_compile_options(-w1${i})
    endforeach()

    add_compile_options(-W4)
endif()

add_compile_options(-MT -Zi -Zf -Zo -bigobj -cgthreads1 -vd0)
add_link_options(-cgthreads:1)

set(_CFLAGS "")
set(_CXXFLAGS "")
set(_CFLAGS_RELEASE "-O2 -O2it -Oy- -Ob3 -fp:fast -GS- -GF -GL -Gw -Gy -arch:SSE2 -GR-")
set(_CFLAGS_DEBUG "-GS -sdl -Gs -guard:cf")
set(_CXXFLAGS_RELEASE "${_CFLAGS_RELEASE}")
set(_CXXFLAGS_DEBUG "${_CFLAGS_DEBUG}")

set(_LDFLAGS_RELEASE "-OPT:REF -OPT:ICF=10 -LTCG:INCREMENTAL -DEBUG:FULL")
set(_LDFLAGS_DEBUG "-DEBUG:FULL")
add_link_options(-DEBUGTYPE:CV,PDATA,FIXUP)

set(_LDFLAGS_STATIC "")
set(_LDFLAGS_STATIC_RELEASE "-LTCG:INCREMENTAL")
set(_LDFLAGS_STATIC_DEBUG "")

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE STRING "" FORCE)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "" FORCE)
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
