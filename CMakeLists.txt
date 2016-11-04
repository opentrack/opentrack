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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include(opentrack-check-build-directory)
include(opentrack-build)

project(opentrack)
cmake_minimum_required(VERSION 2.8.11)

set_property(GLOBAL PROPERTY opentrack-all-modules "")
set_property(GLOBAL PROPERTY opentrack-all-source-dirs "")
set(opentrack-all-translations pl_PL ru_RU)

include(opentrack-policy)
include(opentrack-word-size)
include(opentrack-hier)
include(opentrack-qt)
include(opentrack-platform)
include(opentrack-boilerplate)
include(opentrack-version)
include(opentrack-install)

if(WIN32)
    enable_language(RC)
endif()

add_custom_target(mrproper COMMAND cmake -P "${CMAKE_SOURCE_DIR}/cmake/opentrack-clean-build-directory.cmake" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

set(C CMakeLists.txt)
file(GLOB opentrack-subprojects
    "tracker-*/${C}"
    "proto-*/${C}"
    "filter-*/${C}"
    "options/${C}"
    "api/${C}"
    "compat/${C}"
    "logic/${C}"
    "dinput/${C}"
    "gui/${C}"
    "x-plane-plugin/${C}"
    "csv/${C}"
    "pose-widget/${C}"
    "spline-widget/${C}"
    "qxt-mini/${C}"
    "macosx/${C}"
    "cv/${C}"
    "migration/${C}"
)

list(SORT opentrack-subprojects)

foreach(i ${opentrack-subprojects})
    get_filename_component(i ${i} DIRECTORY)
    add_subdirectory(${i})
endforeach()

merge_translations()
