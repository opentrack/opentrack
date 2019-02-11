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

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_C_LINKER_PREFERENCE_PROPAGATES OFF)
set(CMAKE_CXX_LINKER_PREFERENCE_PROPAGATES ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
include(opentrack-policy NO_POLICY_SCOPE)

project(opentrack)
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(POLICY CMP0083)
    cmake_policy(SET CMP0083 NEW)
    include(CheckPIESupported)
    check_pie_supported()
endif()

include(CMakeDetermineCCompiler)
include(CMakeDetermineCXXCompiler)
include(CMakeParseArguments)

include(opentrack-load-user-settings)
include(opentrack-mrproper)

set_property(GLOBAL PROPERTY opentrack-all-modules "")
set_property(GLOBAL PROPERTY opentrack-all-source-dirs "")

set(opentrack_all-translations "nl_NL;ru_RU;stub;zh_CN")

include(opentrack-hier)
include(opentrack-platform)
include(opentrack-i18n)
include(opentrack-boilerplate)

include(opentrack-qt)
include(opentrack-version)
include(opentrack-variant)

if(WIN32)
    enable_language(RC)
endif()

add_custom_target(mrproper
    COMMAND "${CMAKE_COMMAND}" -P
        "${CMAKE_SOURCE_DIR}/cmake/mrproper.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

function(otr_add_subdirs)
    otr_dist_select_variant()
    get_property(_globs GLOBAL PROPERTY opentrack-subprojects)
    otr_add_target_dirs(_globbed ${_globs})

    foreach(k ${_globbed})
        get_filename_component(k "${k}" DIRECTORY)
        add_subdirectory("${k}")
    endforeach()
endfunction()

otr_add_subdirs()
otr_merge_translations()

#install_sources()

include(opentrack-install)


string(TOUPPER __build_type "${CMAKE_BUILD_TYPE}")
message(STATUS "-- Compile flags:")
#foreach(j C CXX)
foreach(j CXX)
    foreach(i "" "_${__build_type}")
        message(STATUS " ${j}${i}: ${CMAKE_${j}_FLAGS${i}}")
    endforeach()
endforeach()

message(STATUS "-- Link flags:")
foreach(j "" "_${__build_type}")
    #foreach(i EXE SHARED)
    foreach(i SHARED)
        message(STATUS " LINK_${i}${j}: ${CMAKE_${i}_LINKER_FLAGS${j}}")
    endforeach()
endforeach()

message(STATUS "-- Static archive flags:")
foreach(k "" "_${__build_type}")
    message(STATUS " STATIC${k}: ${CMAKE_STATIC_LINKER_FLAGS${k}}")
endforeach()
message(STATUS "--")

