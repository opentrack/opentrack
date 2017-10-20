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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(opentrack-check-build-directory)
include(opentrack-mrproper)

project(opentrack)
cmake_minimum_required(VERSION 2.8.11)
include(opentrack-policy NO_POLICY_SCOPE)
include(opentrack-load-user-settings)

set_property(GLOBAL PROPERTY opentrack-all-modules "")
set_property(GLOBAL PROPERTY opentrack-all-source-dirs "")
set(opentrack_all-translations "" CACHE STRING "Leave empty for default")

if(".${opentrack_all-translations}" STREQUAL ".")
    set(opentrack_all-translations "nl_NL;ru_RU;stub")
endif()

include(opentrack-word-size)
include(opentrack-hier)
include(opentrack-platform)
include(opentrack-i18n)
include(opentrack-boilerplate)

include(opentrack-qt)
include(opentrack-version)
include(opentrack-install)

if(WIN32)
    enable_language(RC)
endif()

add_custom_target(mrproper COMMAND
    "${CMAKE_COMMAND}" -P
    "${CMAKE_SOURCE_DIR}/cmake/opentrack-clean-build-directory.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

otr_add_target_dirs(opentrack-subprojects
    "tracker-*"
    "proto-*"
    "filter-*"
    "ext-*"
    "options"
    "api"
    "compat"
    "logic"
    "dinput"
    "gui"
    "x-plane-plugin"
    "csv"
    "pose-widget"
    "spline"
    "qxt-mini"
    "macosx"
    "cv"
    "migration")

foreach(k ${opentrack-subprojects})
    get_filename_component(k "${k}" DIRECTORY)
    add_subdirectory("${k}")
endforeach()

otr_merge_translations()
install_sources()
